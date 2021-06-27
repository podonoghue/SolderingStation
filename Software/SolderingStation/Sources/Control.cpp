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
   GpioSpare2::setOutput();
   GpioSpare3::setOutput();

   Ch1Drive::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);
   Ch2Drive::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   ch1.controller.setParameters(PID_INTERVAL*10*ms, MIN_DUTY , MAX_DUTY);
   ch2.controller.setParameters(PID_INTERVAL*10*ms, MIN_DUTY , MAX_DUTY);

   ch1.setUpperLimit(MAX_DUTY);
   ch2.setUpperLimit(MAX_DUTY);

   ch1.setDutyCycle(0);
   ch2.setDutyCycle(0);

   ch1.enable();
   ch2.enable();

   Ch1SelectedLed::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);
   Ch2SelectedLed::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

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
   unsigned retry = 10;
   while ((ADConverter::calibrate() != E_NO_ERROR) && (retry-->0)) {
      console.WRITE("ADC calibration failed, retry #").writeln(retry);
   }
   ADConverter::setAveraging(AdcAveraging_32);
   ADConverter::setCallback(adc_cb);
   ADConverter::enableNvicInterrupts(NvicPriority_Normal);

   static auto zx_cb = [](CmpStatus){
      This->zeroCrossingHandler();
   };

   static constexpr uint8_t DAC_THRESHOLD = 2*(Cmp0::MAXIMUM_DAC_VALUE/ADC_REF_VOLTAGE);

   Pit::configureIfNeeded();

   ControlTimerChannel::enableNvicInterrupts(NvicPriority_Normal);

   ZeroCrossingComparator::configure(CmpPower_HighSpeed, CmpHysteresis_1, CmpPolarity_Noninverted);
   ZeroCrossingComparator::setInputFiltered(CmpFilterSamples_7, CmpFilterClockSource_BusClock, 255);
   ZeroCrossingComparator::setInputs();
   ZeroCrossingComparator::configureDac(DAC_THRESHOLD, CmpDacSource_Vdda);
   ZeroCrossingComparator::selectInputs(Cmp0Input_Ptc7, Cmp0Input_Cmp0Dac);
   ZeroCrossingComparator::setCallback(zx_cb);
   ZeroCrossingComparator::enableInterrupts(CmpInterrupt_Falling);
   ZeroCrossingComparator::enableNvicInterrupts(NvicPriority_Normal);

   static auto overcurrent_cb = [](uint32_t){
      This->overCurrentHandler();
   };

   Overcurrent::setInput(PinPull_None, PinAction_IrqFalling, PinFilter_Passive);
   Overcurrent::setCallback(overcurrent_cb);
   Overcurrent::enableNvicInterrupts(NvicPriority_High);
}

/**
 * Check if channel is enabled
 *
 * @param ch Channel to check
 */
bool Control::isEnabled(unsigned ch) {
   Channel &channel = channels[ch];
   return (channel.getState() != ch_off);
}

/**
 * Toggle the enable state of a channel.
 * If becoming enabled it also becomes selected.
 * If becoming disabled the other channel may become selected if enabled.
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
 * It also becomes selected.
 *
 * @param ch Channel to enable
 */
void Control::enable(unsigned ch) {

   Channel &channel = channels[ch];

   channel.setState(ch_active);
   doReportTitle = true;
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
 * Set the selected channel
 *
 * @param ch Channel to select
 */
void Control::setSelectedChannel(unsigned ch) {

   channels.setSelectedChannel(ch);
}

/**
 * Change temperature of currently selected channel
 *
 * @param delta Amount to change by
 */
void Control::changeTemp(int16_t delta) {

   Channel &channel = channels.getSelectedChannel();

   int targetTemperature = channel.getUserTemperature();

   targetTemperature += delta;
   if (targetTemperature>MAX_TEMP) {
      targetTemperature = MAX_TEMP;
   }
   if (targetTemperature<MIN_TEMP) {
      targetTemperature = MIN_TEMP;
   }
   channel.setUserTemperature(targetTemperature);
}

/**
 * Update the current preset from the current temperature of the currently selected channel
 */
void Control::updatePreset() {
   Channel &channel = channels.getSelectedChannel();

   channel.updatePresetTemperature();
}

/**
 * Interrupt handler for over current comparator
 */
void Control::overCurrentHandler() {

   // Mark channels as overloaded
   channels[1].setOverload();
   channels[2].setOverload();

   setNeedsRefresh();
}

/**
 * Interrupt handler for mains zero crossing Comparator
 * This uses the timer to schedule the switchHandler().
 * Occurs @100Hz or 120Hz ~ 10ms or 8.3ms
 * Worse case execution time ~170us.
 */
void Control::zeroCrossingHandler() {

   // Schedule switchOnHandler()
   static auto cb = [](){
      This->switchOnHandler();
   };
   if (!sequenceBusy) {
      // For debug when single stepping only do if previous sequence completed!
      sequenceBusy = true;

      ControlTimerChannel::oneShotInMicroseconds(cb, POWER_ON_DELAY);
   }

   GpioSpare2::set();

   // Counter to initiate screen refresh
   static unsigned refreshCount = 0;

   if (refreshCount++>=50) {
      refreshCount = 0;
      // Update the display regularly
      setNeedsRefresh();
   }

   // Counter to initiate pid update
   static unsigned pidCount = 0;

   // Counter to initiate pid reporting
   static unsigned reportCount = 0;

   if (++pidCount >= PID_INTERVAL) {
      pidCount = 0;

      // Update averages and run PID
      channels[1].upDateCurrentTemperature();

      doSampleCh1 = true;

      if (++reportCount >= 2) {
         reportCount = 0;
         doReport = true;
      }
   }
   else if (pidCount == PID_INTERVAL/2) {
      // Update averages and run PID
      channels[2].upDateCurrentTemperature();

      doSampleCh2 = true;
   }
   GpioSpare2::clear();
}

/**
 * Timer interrupt handler for turning on heaters.
 * It also uses the timer to schedule the sampleHandler().
 */
void Control::switchOnHandler() {

   GpioSpare2::set();

   // Schedule sampleHandler()
   static auto cb = [](){
      This->sampleHandler();
   };
   ControlTimerChannel::oneShotInMicroseconds(cb, SAMPLE_DELAY);

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   // Enable drive to heaters as needed
   if (ch1.isOn()) {
      Ch1Drive::write(true);
   }
   if (ch2.isOn()) {
      Ch2Drive::write(true);
   }

   Ch1SelectedLed::write(ch1.isRunning());
   Ch2SelectedLed::write(ch2.isRunning());

   GpioSpare2::clear();
}

/**
 * Timer interrupt handler for starting ADC sample sequence.
 * It also uses the timer to schedule the switchOffHandler().
 */
void Control::sampleHandler() {

   // Schedule switchOffHandler()
   static auto cb = [](){
      This->switchOffHandler();
   };
   ControlTimerChannel::oneShotInMicroseconds(cb, POWER_OFF_DELAY);

   GpioSpare2::set();

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   // Measure channel 1 if idle this cycle
   if (!ch1.isOn() && doSampleCh1) {
      doSampleCh1 = false;
      adcChannelMask |= (1<<Ch1ColdJunctionNtc::CHANNEL) | (1<<Ch1TipThermocouple::CHANNEL);
   }
   // Measure channel 2 if idle this cycle
   if (!ch2.isOn() && doSampleCh2) {
      doSampleCh2 = false;
      adcChannelMask |= (1<<Ch2ColdJunctionNtc::CHANNEL) | (1<<Ch2TipThermocouple::CHANNEL);
   }

   // Always do chip temperature
   // This also starts the entire sequence of chained conversions
   ChipTemperature::startConversion(AdcInterrupt_Enabled);

   ch1.advance();
   ch2.advance();

   GpioSpare2::clear();
}

/**
 * Timer interrupt handler for turning off the heaters.
 */
void Control::switchOffHandler() {

   GpioSpare2::set();

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   // Disable drive to heaters as needed
   if (!ch1.isOn()) {
      Ch1Drive::write(false);
   }
   if (!ch2.isOn()) {
      Ch2Drive::write(false);
   }

   sequenceBusy = false;

   GpioSpare2::clear();
}

/**
 * Interrupt handler for ADC conversions
 *
 * @param[in] result  Conversion result from ADC channel
 * @param[in] channel ADC channel providing the result
 *
 *   Initial conversion is started from a timer call-back when a channel has an idle cycle.
 *   Several consecutive conversions are then chained in sequence.
 */
void Control::adcHandler(uint32_t result, int adcChannel) {

   GpioSpare2::set();

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   switch (adcChannel) {
      case Ch1ColdJunctionNtc::CHANNEL :
         ch1.coldJunctionTemperature.accumulate(result);
         break;
      case Ch1TipThermocouple::CHANNEL :
         ch1.tipTemperature.accumulate(result);
         break;
      case Ch2ColdJunctionNtc::CHANNEL :
         ch2.coldJunctionTemperature.accumulate(result);
         break;
      case Ch2TipThermocouple::CHANNEL :
         ch2.tipTemperature.accumulate(result);
         break;
      case ChipTemperature::CHANNEL :
         chipTemperature.accumulate(result);
         break;
      default:
         usbdm_assert(false,"Impossible ADC channel");
         break;
   }

   // Set up next conversion as needed
   int nextAdcChannel = __builtin_ffs(adcChannelMask)-1;
   if (nextAdcChannel >= 0) {
      ADConverter::startConversion(AdcInterrupt_Enabled|nextAdcChannel);
   }

   // Mark done current conversion
   // Note - this means each conversion is done twice except for chip temperature
   adcChannelMask &= ~(1<<adcChannel);

   GpioSpare2::clear();
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
      console.setFloatFormat(2, Padding_LeadingSpaces, 2);
      float tipV   = 1000*ch.tipTemperature.getVoltage();
      float tipT   = ch.tipTemperature.getTemperature();
      float coldT  = ch.coldJunctionTemperature.getTemperature();

      console.
      write("Tip = ").write(tipT+coldT).
      write(" (").write(tipT).write("+").write(+coldT).
      write("),(").write(tipV).write(" mV").
      write(",").write(ch.coldJunctionTemperature.getResistance()).write(" ohms)").
      write(" ").write(tipV).write(" ").write(coldT).write(" ").
      //         write(" C, Ch1 Cold = ").write(ch1ColdJunctionTemperature.getConvertedValue()).
      //         write(" C, Ch2 Tip  = ").write(ch2TipTemperature.getConvertedValue()+ch2ColdJunctionTemperature.getConvertedValue()).
      //         write(" C, Ch2 Cold = ").write(ch2ColdJunctionTemperature.getConvertedValue()).
      //         write(" C, Chip = ").write(chipTemperature.getConvertedValue()).
      writeln();
      console.resetFormat();
   }
}

/**
 * Debugging code
 */
void Control::reportPid(Channel &ch) {

   doReport = false;

   console.setFloatFormat(2, Padding_LeadingSpaces, 2);
   if (doReportTitle) {
      doReportTitle = false;
      console.
         write("Kp = ").
         write(ch.getTip()->getKp()).
         write(',').
         write("Drive").write(',').
         write("Temp").write(',').
         write("Error").write(',').
         write("P").write(',').
         write("I").write('<').write(ch.getTip()->getILimit()).write(',').
         write("D").writeln();
      console.
         write("Ki = ").
         write(ch.getTip()->getKi()).
         write(',').
         writeln(ch.getTipName());
      console.
         write("Kd = ").writeln(ch.getTip()->getKd());
   }
   if (ch.controller.isEnabled()) {
      double time  = ch.controller.getElapsedTime();
      float input  = ch.controller.getInput();
      float output = ch.controller.getOutput();

      console.write(time).write(", ").write(output).write(", ").write(input);
      ch.controller.report();
      console.resetFormat();
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
      if (doReport) {
         reportPid(channels[1]);
         //         reportChannel(channels[1]);
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
            setSelectedChannel(1);
            toggleEnable(1);
            break;

         case ev_Ch2Hold      :
            setSelectedChannel(2);
            toggleEnable(2);
            break;

         case ev_Ch1Ch2Hold   :
            if (!isEnabled(1) && !isEnabled(2)) {
               // Both currently off - turn on both channels
               enable(2);
               enable(1); // Active channel
            }
            else {
               // >= 1 channel on - turn both off
               disable(1);
               disable(2);
            }
            break;

         case ev_Ch1Release      :
            setSelectedChannel(1);
            break;

         case ev_Ch2Release      :
            setSelectedChannel(2);
            break;

         case ev_QuadRelease   :
            updatePreset();
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
