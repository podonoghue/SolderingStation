/*
 * Control.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include <stdlib.h>
#include "hardware.h"
#include "smc.h"
#include "SwitchPolling.h"
#include "Control.h"
#include "Channels.h"
#include "SettingsData.h"
#include "BoundedInteger.h"
#include "Menus.h"
#include "wdog.h"

using namespace USBDM;

class DebLed {
public:
   DebLed() {
      Debug::high();
   }
   ~DebLed() {
      Debug::low();
   }
};

/**
 * Initialise the control
 */
void Control::initialise() {
   using namespace USBDM;

   Debug::setOutput();

   // Static function to use as ADC call-back
   static AdcCallbackFunction adc_cb = [](uint32_t result, int channel){
      control.adcHandler(result, channel);
   };

   FixedGainAdc::configure(
         ADC_RESOLUTION,
         AdcClockSource_Bus,
         AdcSample_20,
         AdcPower_Normal,
         AdcMuxsel_B,
         AdcClockRange_Normal,
         AdcAsyncClock_Disabled);

   FixedGainAdc::setReference(AdcRefSel_VrefHL);

   // Calibrate ADC
   unsigned retry = 10;
   while ((FixedGainAdc::calibrate() != E_NO_ERROR) && (retry-->0)) {
      console.WRITE("ADC calibration failed, retry #").WRITELN(retry);
   }
   FixedGainAdc::setAveraging(AdcAveraging_8);
   FixedGainAdc::setCallback(adc_cb);
   FixedGainAdc::enableNvicInterrupts(NvicPriority_MidHigh);

   ProgrammableGainAdc::configure(
         ADC_RESOLUTION,
         AdcClockSource_Bus,
         AdcSample_20,
         AdcPower_Normal,
         AdcMuxsel_B,
         AdcClockRange_Normal,
         AdcAsyncClock_Disabled);

   ProgrammableGainAdc::setReference(AdcRefSel_VrefOut);

   // Calibrate ADC
   retry = 10;
   while ((ProgrammableGainAdc::calibrate() != E_NO_ERROR) && (retry-->0)) {
      console.WRITE("ADC calibration failed, retry #").WRITELN(retry);
   }
   ProgrammableGainAdc::setAveraging(AdcAveraging_8);
   ProgrammableGainAdc::setCallback(adc_cb);
   ProgrammableGainAdc::enableNvicInterrupts(NvicPriority_MidHigh);
   ProgrammableGainAdc::configurePga(AdcPgaMode_NormalPower, AdcPgaGain_4);

   // Configure PIT for use in timing
   ControlTimerChannel::configureIfNeeded();
   ControlTimerChannel::enableNvicInterrupts(NvicPriority_Normal);

   // Configure comparator for mains zero-crossing detection
   // Trigger is actually on the falling edge of rectified waveform near zero
   static CmpCallbackFunction zero_crossing_cb = [](CmpStatus){
      control.zeroCrossingHandler();
   };

   // Sensitivity of overload amplifier and shunt (V/A)
   static constexpr float OVERLOAD_VOLT_PER_AMP = .05*(1.0+22.0/10.0); // .160 V/A

   // Current limit based on transformer AMPS (160 VA, 24Vrms, 20% overload)
   static constexpr float CURRENT_LIMIT = 1.2 * 1.414 * (160.0/24.0);  // 11.3 A

   static_assert((CURRENT_LIMIT*OVERLOAD_VOLT_PER_AMP)<CMP_REF_VOLTAGE); // ~1.8V

   // Threshold for zero-crossing comparator
   static constexpr uint8_t ZERO_CROSSING_DAC_THRESHOLD =
         (CURRENT_LIMIT*OVERLOAD_VOLT_PER_AMP)*(ZeroCrossingComparator::MAXIMUM_DAC_VALUE/CMP_REF_VOLTAGE);

   ZeroCrossingComparator::configure(CmpPower_HighSpeed, CmpHysteresis_3, CmpPolarity_Noninverted);
   ZeroCrossingComparator::setInputFiltered(CmpFilterSamples_7, CmpFilterClockSource_BusClock, 255);
   ZeroCrossingComparator::setInputs();
   ZeroCrossingComparator::configureDac(ZERO_CROSSING_DAC_THRESHOLD, CmpDacSource_Vdda);
   ZeroCrossingComparator::selectInputs(ZeroCrossingComparator::Input_ZeroCrossingInput, ZeroCrossingComparator::Input_CmpDac);
   ZeroCrossingComparator::setCallback(zero_crossing_cb);
   ZeroCrossingComparator::enableInterrupts(CmpInterrupt_Falling);
   ZeroCrossingComparator::enableNvicInterrupts(NvicPriority_MidHigh);

   // Over-current detection using external comparator and pin IRQ
   static CmpCallbackFunction overcurrent_cb = [](CmpStatus){
      // Mark channels as overloaded
      channels[1].setOverload();
      channels[2].setOverload();
      control.setNeedsRefresh();
   };

   // Threshold for over-current comparator (0.305 V/A) (10Apeak limit ~3.05V)
   static constexpr uint8_t OVERCURRENT_DAC_THRESHOLD = 1.5*(OverCurrentComparator::MAXIMUM_DAC_VALUE/CMP_REF_VOLTAGE);

   OverCurrentComparator::configure(CmpPower_HighSpeed, CmpHysteresis_3, CmpPolarity_Noninverted);
   OverCurrentComparator::setInputFiltered(CmpFilterSamples_7, CmpFilterClockSource_BusClock, 20);
   OverCurrentComparator::setInputs();
   OverCurrentComparator::configureDac(OVERCURRENT_DAC_THRESHOLD, CmpDacSource_Vdda);
   OverCurrentComparator::selectInputs(OverCurrentComparator::Input_Overcurrent, OverCurrentComparator::Input_CmpDac);
   OverCurrentComparator::setCallback(overcurrent_cb);
   OverCurrentComparator::enableInterrupts(CmpInterrupt_Rising);
   OverCurrentComparator::enableNvicInterrupts(NvicPriority_High);

   // Set up multiplexor for measurements
   AmplifierControl::setOutput(PinDriveStrength_Low, PinDriveMode_PushPull, PinSlewRate_Slow);

   // Enable amplifier input clamp
   Clamp::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

   static auto wdogCallback = []() {
      ch1Drive.write(0b00);
      ch2Drive.write(0b00);
      ch1Drive.setInput();
      ch2Drive.setInput();
   };

   Wdog::setCallback(wdogCallback);
   Wdog::setTimeout(20*ms);
   Wdog::configure(
         WdogEnable_Enabled,
         WdogClock_Lpo,
         WdogWindow_Disabled,
         WdogInterrupt_Enabled,
         WdogEnableInDebug_Disabled,
         WdogEnableInStop_Enabled,
         WdogEnableInWait_Enabled);
   Wdog::writeRefresh(0xA602, 0xB480);
}

/**
 * Toggle the enable state of a channel.
 *
 * @param chNum Channel to modify
 */
void Control::toggleEnable(unsigned chNum) {
   Channel &channel = channels[chNum];
   if (channel.getState() == ch_off) {
      enable(chNum);
   }
   else {
      disable(chNum);
   }
}

/**
 * Enable channel.
 *
 * @param chNum Channel to enable
 */
void Control::enable(unsigned chNum) {

   Channel &channel = channels[chNum];

   channel.setState(ch_active);
   doReportPidTitle = true;
   reportCount      = 0;
}

/**
 * Disable channel.
 * The other channel may become selected if enabled.
 *
 * @param chNum Channel to disable
 */
void Control::disable(unsigned chNum) {

   Channel &channel = channels[chNum];

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

/// Extra settling time on initial measurement or when changing gain
static constexpr unsigned INITAL_SAMPLE_DELAY     = 0;

/// Settling time for high gain amplifier
static constexpr unsigned HIGH_GAIN_SAMPLE_DELAY  = 200;

/// Settling time for low gain amplifier
static constexpr unsigned LOW_GAIN_SAMPLE_DELAY   = 100;

/**
 * Comparator interrupt handler for controlling the heaters.
 * This is triggered just prior to the mains zero-crossing.
 * It also uses the timer to schedule the ADC sampling.
 */
void Control::zeroCrossingHandler() {

   DebLed xx;

   if (holdOff) {
       return;
   }

   holdOff = true;

//   // Schedule ADC conversions
//   static PitCallbackFunction cb = [](){
//      DebLed xx;
//      // Do chip temperature measurement
//      // This also starts the entire sequence of chained conversions
//      // This is delayed until after the thermocouple amplifier has recovered
//      // from being over-driven during the previous cycle.
//      ChipTemperatureAdcChannel::startConversion(AdcInterrupt_Enabled);
//   };
//   ControlTimerChannel::oneShotInMicroseconds(cb, INITAL_SAMPLE_DELAY);
//
//   Debug::set();

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   // Turn off drives
   ch1Drive.write(0b00);
   ch2Drive.write(0b00);

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

   // Get measurements to do
   int sequenceLength = 0;
   sequenceLength += ch1.getMeasurementSequence(sequence+sequenceLength, CH1_MASK);
   sequenceLength += ch2.getMeasurementSequence(sequence+sequenceLength, CH2_MASK);
   sequence[sequenceLength] = MuxSelect_Complete;

   // Used to sort measurements according to AMPLIFIER and BIAS settings
   // Avoids enabling/disabling bias multiple times
   static auto comp = [](const void *p1, const void *p2) {
      const MuxSelect *left  = static_cast<const MuxSelect*>(p1);
      const MuxSelect *right = static_cast<const MuxSelect*>(p2);
      return ((*right^BIAS_MASK)&(AMPLIFIER_MASK|BIAS_MASK))-((*left^BIAS_MASK)&(AMPLIFIER_MASK|BIAS_MASK));
   };

   // Sort (excluding sentinel value) so that:
   // - High-gain measurements are done first. This prevents saturation of the high gain amplifier.
   // - Bias is only changed once in sequence
   qsort(sequence, sequenceLength, sizeof(sequence[0]), comp);

   // Restart sequence
   sequenceIndex = 0;

   // Set up initial measurement arrangement (including bias and gain)
   AmplifierControl::write(sequence[0]);

   // Do chip temperature measurement
   // This also starts the entire sequence of chained conversions
   ChipTemperatureAdcChannel::startConversion(AdcInterrupt_Enabled);
}

/**
 * Interrupt handler for ADC conversions
 *
 * @param [in] result     Conversion result from ADC channel
 * @param [in] adcChannel ADC channel providing the result
 *
 *   Initial conversion is started from zeroCrossingHandler().
 *   Several consecutive conversions are then chained in sequence.
 */
void Control::adcHandler(uint32_t result, int adcChannel) {

   // Conversion started last time == result to process this time
   static MuxSelect lastConversion = MuxSelect_Complete;

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   DebLed db;

   Clamp::on();

   if (adcChannel == ChipTemperatureAdcChannel::CHANNEL) {
      // First conversion in sequence
      // Process chip temperature
      chipTemperatureAverage.accumulate(result);

      // Unclamp amplifier inputs (mux output)
      // Delayed to here to allow drive to drop
      Clamp::off();
   }
   else {
      switch(lastConversion&(CHANNEL_MASK|AB_MASK)) {
         // For debug breakpoints
         case CH1_MASK|CHA_MASK :
            __asm__("nop");
            break;
         case CH1_MASK|CHB_MASK :
            __asm__("nop");
            break;
         case CH2_MASK|CHA_MASK :
            __asm__("nop");
            break;
         case CH2_MASK|CHB_MASK :
            __asm__("nop");
            break;
         default:
            __asm__("nop");
            break;
      }
      // Process last measurement - pass measurement to correct channel
      uint8_t lastChannelUsed = lastConversion & CHANNEL_MASK;
      if (lastChannelUsed == CH1_MASK) {
         ch1.processMeasurement(lastConversion, result);
      }
      else {
         ch2.processMeasurement(lastConversion, result);
      }
   }
   // Get conversion to start this cycle
   MuxSelect currentConversion = sequence[sequenceIndex++];

   if (currentConversion == MuxSelect_Complete) {
      // Completed all thermocouple/thermistor conversion sequences

      // Clamp amplifier input to prevent op-amp saturation
      Clamp::on();

      // Run PID and update drive
      pidHandler();

      // Allow new sequence
      holdOff = false;
      // Pat the watchdog
      Wdog::writeRefresh(0xA602, 0xB480);

      return;
   }

   // Set up multiplexor, bias and gain boost in external hardware
   AmplifierControl::write(currentConversion);

   // Need extra time if bias has changed
//   bool biasHasChange = (currentConversion ^ lastConversion) & BIAS_MASK;

   // Allow extra time for high gain amplifier to settle
   bool isHighGainAmplifier = (currentConversion & AMPLIFIER_MASK);

   // Allow extra settling time on 1st sample
   unsigned delay = (sequenceIndex == 1)?INITAL_SAMPLE_DELAY:0;

   if (isHighGainAmplifier) {
      // Set up PGA gain
      ProgrammableGainAdc::configurePga(AdcPgaMode_NormalPower, muxPgaGain(currentConversion));

      // Longer settling time for high-gain amplifier
      delay += HIGH_GAIN_SAMPLE_DELAY;
   }
   else {
      // Standard delay
      delay += LOW_GAIN_SAMPLE_DELAY;
   }

   // Longer time for bias turning on
//   delay += biasHasChange?INITAL_SAMPLE_DELAY:0;

   // Last is now current!
   lastConversion = currentConversion;

   // Schedule next ADC conversion
   // This is delayed to allow for mux, bias and amplifier settling time
   static PitCallbackFunction cb = [](){
      DebLed db;
      if (lastConversion&AMPLIFIER_MASK) {
         ProgrammableGainAdcChannel::startConversion(AdcInterrupt_Enabled);
      }
      else {
         FixedGainAdcChannel::startConversion(AdcInterrupt_Enabled);
      }
   };

   Clamp::off();

   ControlTimerChannel::oneShotInMicroseconds(cb, delay);
}

/**
 * Timer interrupt handler for updating PID
 * This includes the heater drives
 */
void Control::pidHandler() {
   Debug::off();

   channels[1].update();
   channels[2].update();

   Debug::on();
}

/**
 * Refresh the display of channel information
 */
void Control::refresh() {
   needRefresh = false;

   if (isDisplayInUse()) {
      // Update display
      display.displayChannels();
   }
   else {
      display.enable(false);
   }
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

   if (ch.isRunning()) {
      ch.report(doReportPidTitle);
      doReportPidTitle = false;
      console.resetFormat();
   }
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

      if (!isDisplayInUse()) {
         // Fix display
         wakeUpDisplay();
         refresh();

         // Ignore event that woke screen
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
            if (channels.getSelectedChannelNumber() == 1) {
               channels[1].nextPreset();
            }
            else {
               channels.setSelectedChannel(1);
            }
            break;

         case ev_Ch2Release      :
            if (channels.getSelectedChannelNumber() == 2) {
               channels[2].nextPreset();
            }
            else {
               channels.setSelectedChannel(2);
            }
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

/**
 * Checks if display is in use
 *
 * @param milliseconds Amount to increment the idle time by
 */
void Control::updateDisplayInUse(unsigned milliseconds) {
   if (channels[1].isToolIdle() && channels[2].isToolIdle()) {
      idleTime += milliseconds;
   }
   else {
      idleTime = 0;
   }
}

void Control::wakeUpDisplay() {
   idleTime = 0;
}

/**
 * Checks is the display is in use
 * Considered in use for 2 minutes after any activity or while either channel is on
 *
 * @return True  Display is needed
 * @return False Display may be turned off
 */
bool Control::isDisplayInUse() {
   static constexpr unsigned DISPLAY_OFF_TIME = 300*1000; // milliseconds

   return (idleTime<DISPLAY_OFF_TIME);
}
