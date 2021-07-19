/*
 * Control.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "smc.h"
#include "SwitchPolling.h"
#include "Control.h"
#include "Channels.h"
#include "SettingsData.h"
#include "BoundedInteger.h"
#include "Menus.h"

using namespace USBDM;

/**
 * Initialise the control
 */
void Control::initialise() {
   using namespace USBDM;

   GpioSpare1::setOutput();
   Debug::setOutput();
   GpioSpare3::setOutput();

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   ch1.setUpperLimit(MAX_DUTY);
   ch2.setUpperLimit(MAX_DUTY);

   ch1.setDutyCycle(0);
   ch2.setDutyCycle(0);

   ch1.enable();
   ch2.enable();

   // Configure ADC to use a call-back
   static auto adc_cb = [](uint32_t result, int channel){
      This->adcHandler(result, channel);
   };
   ADConverter::configure(
         ADC_RESOLUTION,
         AdcClockSource_Bus,
         AdcSample_20,
         AdcPower_Normal,
         AdcMuxsel_B,
         AdcClockRange_Normal,
         AdcAsyncClock_Disabled);

   // Calibrate ADC
   unsigned retry = 10;
   while ((ADConverter::calibrate() != E_NO_ERROR) && (retry-->0)) {
      console.WRITE("ADC calibration failed, retry #").WRITELN(retry);
   }
   ADConverter::setAveraging(AdcAveraging_off);
   ADConverter::setCallback(adc_cb);
   ADConverter::enableNvicInterrupts(NvicPriority_MidHigh);

   // Configure PIT for use in timing
   Pit::configureIfNeeded();
   ControlTimerChannel::enableNvicInterrupts(NvicPriority_Normal);

   // Configure comparator for mains zero-crossing detection
   // Trigger is actually on the falling edge of rectified waveform near zero

   static auto zero_crossing_cb = [](CmpStatus){
      This->zeroCrossingHandler();
   };

   // Threshold for comparator
   static constexpr uint8_t ZERO_CROSSING_DAC_THRESHOLD = 2.8*(ZeroCrossingComparator::MAXIMUM_DAC_VALUE/ADC_REF_VOLTAGE);

   ZeroCrossingComparator::configure(CmpPower_HighSpeed, CmpHysteresis_3, CmpPolarity_Noninverted);
   ZeroCrossingComparator::setInputFiltered(CmpFilterSamples_7, CmpFilterClockSource_BusClock, 255);
   ZeroCrossingComparator::setInputs();
   ZeroCrossingComparator::configureDac(ZERO_CROSSING_DAC_THRESHOLD, CmpDacSource_Vdda);
   ZeroCrossingComparator::selectInputs(ZeroCrossingInput, Cmp0Input_Cmp0Dac);
   ZeroCrossingComparator::setCallback(zero_crossing_cb);
   ZeroCrossingComparator::enableInterrupts(CmpInterrupt_Falling);
   ZeroCrossingComparator::enableNvicInterrupts(NvicPriority_Normal);

   // Over-current detection using external comparator and pin IRQ
   static auto overcurrent_cb = [](uint32_t){
      // Mark channels as overloaded
      channels[1].setOverload();
      channels[2].setOverload();
      This->setNeedsRefresh();
};

   Overcurrent::setInput(PinPull_None, PinAction_IrqFalling, PinFilter_Passive);
   Overcurrent::setCallback(overcurrent_cb);
   Overcurrent::enableNvicInterrupts(NvicPriority_High);
}

/**
 * Toggle the enable state of a channel.
 *
 * @param ch Channel to modify
 */
void Control::toggleEnable(unsigned ch) {
   Channel &channel = channels[ch];
   if (channel.getState() == ch_off) {
      enable(ch);
   }
   else {
      disable(ch);
   }
}

/**
 * Enable channel.
 *
 * @param ch Channel to enable
 */
void Control::enable(unsigned ch) {

   Channel &channel = channels[ch];

   channel.setState(ch_active);
   doReportPidTitle = true;
   reportCount      = 0;
}

/**
 * Disable channel.
 *
 * @param ch Channel to disable
 */
void Control::disable(unsigned ch) {

   Channel &channel = channels[ch];

   channel.setState(ch_off);
}

/**
 * Change temperature of currently selected channel
 *
 * @param delta Amount to change by
 */
void Control::changeTemp(int16_t delta) {

   Channel &channel = channels.getSelectedChannel();

   BoundedInteger targetTemperature(MIN_TEMP, MAX_TEMP, channel.getUserTemperature());

   targetTemperature += delta;

   channel.setUserTemperature(targetTemperature);
}

/// Delay between zero crossing and sampling (ADC conversion start) (us)
static constexpr unsigned SAMPLE_DELAY  = 1000;

/**
 * Comparator interrupt handler for controlling the heaters.
 * This is triggered just prior to the mains zero-crossing.
 * It also uses the timer to schedule the ADC sampling.
 */
void Control::zeroCrossingHandler() {

   // Schedule ADC conversions
   static auto cb = [](){
      // Do chip temperature measurement
      // This also starts the entire sequence of chained conversions
      // This is delayed until after the thermocouple amplifier has recovered
      // from being over-driven during the previous cycle.
      ChipTemperature::startConversion(AdcInterrupt_Enabled);
   };
   ControlTimerChannel::oneShotInMicroseconds(cb, SAMPLE_DELAY);

   // Set up bias resistors for later tip measurements
   Ch1BiasResistor::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Fast);
   Ch1BiasResistor::write(channels[1].measurement->isBiasRequired());
   Ch2BiasResistor::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Fast);
   Ch2BiasResistor::write(channels[2].measurement->isBiasRequired());

//   Debug::set();

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   // Turn off drives
   ch1.driveWrite(false);
   ch2.driveWrite(false);

   // Counter to initiate screen refresh
   static unsigned refreshCount = 0;
   if (refreshCount++ >= REFRESH_INTERVAL) {
      refreshCount = 0;
      // Update the display regularly
      setNeedsRefresh();
   }

   // PID reporting
   if (++reportCount >= PID_LOG_INTERVAL) {
      reportCount = 0;
      doReportPid = true;
   }

//   Debug::clear();
}

/**
 * Interrupt handler for ADC conversions
 *
 * @param[in] result  Conversion result from ADC channel
 * @param[in] channel ADC channel providing the result
 *
 *   Initial conversion is started from zeroCrossingHandler().
 *   Several consecutive conversions are then chained in sequence.
 */
void Control::adcHandler(uint32_t result, int adcChannel) {

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   switch (adcChannel) {

      case ChipTemperature::CHANNEL :
         Debug::set();
         chipTemperature.accumulate(result);
         Ch1ColdJunctionNtc::startConversion(AdcInterrupt_Enabled);
         Debug::clear();
         break;

      case Ch1ColdJunctionNtc::CHANNEL :
         ch1.measurement->auxliliarySensorAccumulate(result);
         Ch2ColdJunctionNtc::startConversion(AdcInterrupt_Enabled);
         break;

      case Ch2ColdJunctionNtc::CHANNEL :
         Debug::set();
         ch2.measurement->auxliliarySensorAccumulate(result);
         Ch1TipThermocouple::startConversion(AdcInterrupt_Enabled);
         break;

      case Ch1TipThermocouple::CHANNEL :
         ch1.measurement->tipSensorAccumulate(result);
         Ch2TipThermocouple::startConversion(AdcInterrupt_Enabled);
         usbdm_assert(!ch1.driveReadState(), "Sample while channel 1 busy");
         break;

      case Ch2TipThermocouple::CHANNEL :
         ch2.measurement->tipSensorAccumulate(result);
         Debug::clear();
         usbdm_assert(!ch2.driveReadState(), "Sample while channel 2 busy");
         // Disable bias while checking IDs
         Ch1BiasResistor::off();
         Ch1Id::setInput();
         Ch2Id::setInput();
         pidHandler();
         Ch1Id::startConversion(AdcInterrupt_Enabled);
         break;

      case Ch1Id::CHANNEL :
         Ch2Id::startConversion(AdcInterrupt_Enabled);
         break;

      case Ch2Id::CHANNEL :
         break;

      default:
         usbdm_assert(false, "Impossible ADC channel");
         break;
   }
}

/**
 * Timer interrupt handler for updating PID
 * This includes the heater drives
 */
void Control::pidHandler() {

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   // Update drive to heaters as needed
   ch1.updateDrive();
   ch2.updateDrive();

   // Calculate current temperatures
   ch1.upDateCurrentTemperature();
   ch2.upDateCurrentTemperature();

   // Update averages and run PID
   ch1.upDatePid();
   ch2.upDatePid();
}

/**
 * Refresh the display of channel information
 */
void Control::refresh() {
   needRefresh = false;

   // Update display
   display.displayChannels();
}

/**
 * Debugging code
 */
void Control::reportChannel(Channel &ch) {
   static int count=0;

   if (count++ == 500) {
      count = 0;
      ch.report();
   }
}

/**
 * Debugging code
 */
void Control::reportPid(Channel &ch) {

   doReportPid = false;

   if (doReportPidTitle) {
      doReportPidTitle = false;
      ch.controller.reportHeading(ch);
   }
   if (ch.controller.isEnabled()) {
      ch.controller.report(ch);
   }
   console.resetFormat();
}

/**
 * Main event loop for front panel events
 */
void Control::eventLoop()  {

   // Make sure to draw screen initially
   setNeedsRefresh();

   for(;;) {
      if (doReportPid) {
         reportPid(channels[1]);
      }
      if (needRefresh) {
         // Redraw screen
         refresh();
      }

      Event event = switchPolling.getEvent();

      if (event.type == ev_None) {
         // Wait for something to happen
         Smc::enterWaitMode();
         continue;
      }

      // Assume visible change due to event
      setNeedsRefresh();

      //         console.write("Position = ").write(event.change).write(", Event = ").writeln(getEventName(event));
      //         console.write("Event = ").writeln(getEventName(event));

      switch(event.type) {
         case ev_Ch1Hold      :
            channels.setSelectedChannel(1);
            toggleEnable(1);
            break;

         case ev_Ch2Hold      :
            channels.setSelectedChannel(2);
            toggleEnable(2);
            break;

         case ev_Ch1Release      :
            channels.setSelectedChannel(1);
            break;

         case ev_Ch2Release      :
            channels.setSelectedChannel(2);
            break;

         case ev_QuadRelease   :
            channels.getSelectedChannel().updatePresetTemperature();
            break;

         case ev_QuadRotate    :
            changeTemp(event.change);
            break;

         case ev_QuadRotatePressed    :
            channels.getSelectedChannel().changeTip(event.change);
            break;

         case ev_SelRelease :
            channels.getSelectedChannel().nextPreset();
            break;

         case ev_SelHold :
         case ev_QuadHold :
            disable(1);
            disable(2);

            // Needed to immediately update the LEDs
            refresh();

            // Change to settings sub-menu
            Menus::settingsMenu();
            break;
         default: break;
      }
   }
}

/// this pointer for static members (call-backs)
Control *Control::This = nullptr;
