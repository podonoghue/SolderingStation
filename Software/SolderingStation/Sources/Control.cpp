/*
 * Control.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#include "Control.h"
#include "flash.h"
#include "SwitchPolling.h"
#include "smc.h"

using namespace USBDM;

__attribute__ ((section(".flexRAM")))
NonvolatileArray<int, 3> ch1Settings;

__attribute__ ((section(".flexRAM")))
NonvolatileArray<int, 3> ch2Settings;

/// Default temperature presets
static const int defaultTemperaturePresets[3] = {300, 350, 360};

/**
 * A derived class similar to this should be created to do the following:
 * - Wait for initialisation of the FlexRAM from the Flash backing store - Flash();
 * - Configure and partition the flash on the first reset after programming the device - initialiseEeprom().
 * - Do once-only initialisation of non-volatile variables when the above occurs.
 */
class NvInit : public Flash {
public:
   NvInit() : Flash() {

      // Initialise the non-volatile system and configure if necessary
      volatile FlashDriverError_t rc = initialiseEeprom<EepromSel_2KBytes, PartitionSel_flash0K_eeprom32K, SplitSel_disabled>();

      if (rc == FLASH_ERR_NEW_EEPROM) {
         // This is the first reset after programming the device
         // Initialise the non-volatile variables as necessary
         // If not initialised they will have an initial value of 0xFF
         ch1Settings = defaultTemperaturePresets;
         ch2Settings = defaultTemperaturePresets;
         console.writeln("Initialising NV variables");
      }
      else {
         usbdm_assert(rc != FLASH_ERR_OK, "FlexNVM initialisation error");
         console.writeln("Not initialising NV variables");
      }
   }
};

/**
 * Constructor
 * Does minimal work - see initialise for main initialisation.
 */
Control::Control() {

   This = this;

   // Initialise non-volatile storage
   NvInit nvinit;
}

/**
 * Initialise the control
 */
void Control::initialise() {
   using namespace USBDM;

   GpioSpare1::setOutput();
   GpioSpare2::setOutput();
   GpioSpare3::setOutput();

   ch1DutyCycleCounter.setUpperLimit(40);
   ch2DutyCycleCounter.setUpperLimit(40);

   Ch1Drive::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);
   Ch2Drive::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

   ch1DutyCycleCounter.setDutyCycle(0);
   ch2DutyCycleCounter.setDutyCycle(0);

   ch1DutyCycleCounter.enable();
   ch2DutyCycleCounter.enable();

   Ch1SelectedLed::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);
   Ch2SelectedLed::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

   Ch1ActiveLed::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);
   Ch2ActiveLed::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

   static auto adc_cb = [](uint32_t result, int channel){
      This->adcHandler(result, channel);
   };
   Adc0::configure(
         ADC_RESOLUTION,
         AdcClockSource_Bus,
         AdcSample_20,
         AdcPower_Normal,
         AdcMuxsel_B,
         AdcClockRange_Normal,
         AdcAsyncClock_Disabled);
   unsigned retry = 10;
   while ((Adc0::calibrate() != E_NO_ERROR) && (retry-->0)) {
      console.write("ADC calibration failed, retry #").writeln(retry);
   }
   Adc0::setAveraging(AdcAveraging_32);
   Adc0::setCallback(adc_cb);
   Adc0::enableNvicInterrupts(NvicPriority_Normal);

   channels[0].targetTemperature = ch1Settings[1];
   channels[0].preset            = 1;
   channels[1].targetTemperature = ch2Settings[1];
   channels[1].preset            = 1;

   static auto zx_cb = [](CmpStatus){
      This->zeroCrossingHandler();
   };

   static constexpr uint8_t DAC_THRESHOLD = (3.3/2)*(Cmp0::MAXIMUM_DAC_VALUE/ADC_REF_VOLTAGE);

   Pit::configureIfNeeded();
//   timerChannel = Pit::allocateChannel();
   PitChannel::enableNvicInterrupts(NvicPriority_Normal);

   ZeroCrossingComparator::configure(CmpPower_HighSpeed, CmpHysteresis_1, CmpPolarity_Noninverted);
   ZeroCrossingComparator::setInputFiltered(CmpFilterSamples_7, CmpFilterClockSource_BusClock, 255);
   ZeroCrossingComparator::setInputs();
   ZeroCrossingComparator::configureDac(DAC_THRESHOLD, CmpDacSource_Vdda);
   ZeroCrossingComparator::selectInputs(Cmp0Input_Ptc7, Cmp0Input_Cmp0Dac);
   ZeroCrossingComparator::setCallback(zx_cb);
   ZeroCrossingComparator::enableInterrupts(CmpInterrupt_Both);
   ZeroCrossingComparator::enableNvicInterrupts(NvicPriority_Normal);

   static auto overcurrent_cb = [](uint32_t){
      This->overCurrentHandler();
   };

   Overcurrent::setInput(PinPull_None, PinAction_IrqFalling, PinFilter_Passive);
   Overcurrent::setCallback(overcurrent_cb);
   Overcurrent::enableNvicInterrupts(NvicPriority_High);

   ch1DutyCycleCounter.enable();
   ch2DutyCycleCounter.enable();
}

/**
 * Check if channel is enabled
 *
 * @param ch Channel to check
 */
bool Control::isEnabled(unsigned ch) {
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];
   return (channel.state != ch_off);
}

/**
 * Toggle the enable state of a channel.
 * If becoming enabled it also becomes selected.
 * If becoming disabled the other channel may become selected if enabled.
 *
 * @param ch Channel to modify
 */
void Control::toggleEnable(unsigned ch) {
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];
   if (channel.state == ch_off) {
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
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];
   channel.state = ch_active;
   // Make this channel selected
   selectedChannel = ch;
}

/**
 * Disable channel.
 * The other channel may become selected if enabled.
 *
 * @param ch Channel to disable
 */
void Control::disable(unsigned ch) {
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];

   channel.state = ch_off;
   unsigned otherChannel = 3-ch;
   selectedChannel = 0;
   if (channels[otherChannel-1].state != ch_off) {
      // Make other channel selected
      selectedChannel = otherChannel;
   }
}

/**
 * Backoff channel (if enabled).
 *
 * @param ch Channel to modify
 */
void Control::backOff(unsigned ch) {
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];
   if (channel.state == ch_off) {
      // Ignore
      return;
   }
   channel.state = ch_backoff;
}

/**
 * Wake-up channel (if in back-off).
 * It also becomes selected.
 *
 * @param ch Channel to modify
 */
void Control::wakeUp(unsigned ch) {
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];
   if (channel.state == ch_backoff) {
      enable(ch);
   }
}

/**
 * Set the selected channel
 *
 * @param ch Channel to select
 */
void Control::setSelectedChannel(unsigned ch) {
   usbdm_assert((ch == 1)||(ch == 2), "Illegal channel");
   Channel &channel = channels[ch-1];
   if (channel.state == ch_off) {
      // Can't make off channel selected
      return;
   }
   selectedChannel = ch;
}

/**
 * Change the temperature to the next preset value for the currently selected channel
 */
void Control::nextPreset() {
   if (selectedChannel == 0) {
      return;
   }
   Channel &channel = channels[selectedChannel-1];
   NonvolatileArray<int, 3> &chSettings = (selectedChannel==1)?ch1Settings:ch2Settings;

   channel.preset++;
   if (channel.preset>=NUM_PRESETS) {
      channel.preset = 0;
   }
   channel.targetTemperature = chSettings[channel.preset];
   channel.modified          = false;
}

/**
 * Change temperature of currently selected channel
 *
 * @param delta Amount to change by
 */
void Control::changeTemp(int16_t delta) {

   Channel &channel                     = channels[0];

   int &currentTemp = channel.targetTemperature;

   {
      // Dummy code
      currentTemp += delta;
      if (currentTemp>100) {
         currentTemp = 100;
      }
      if (currentTemp<0) {
         currentTemp = 0;
      }
      ch1DutyCycleCounter.setDutyCycle(currentTemp);
   }


   //   if (selectedChannel == 0) {
   //      return;
   //   }
   //   Channel &channel                     = channels[selectedChannel-1];
   //   NonvolatileArray<int, 3> &chSettings = (selectedChannel==1)?ch1Settings:ch2Settings;
   //
   //   if (channel.state == ch_off) {
   //      return;
   //   }
   //   int &currentTemp = channel.targetTemperature;
   //
   //   currentTemp += delta;
   //   if (currentTemp>MAX_TEMP) {
   //      currentTemp = MAX_TEMP;
   //   }
   //   if (currentTemp<MIN_TEMP) {
   //      currentTemp = MIN_TEMP;
   //   }
   //   channel.modified = (currentTemp != chSettings[channel.preset]);
}

/**
 * Update the current preset from the current temperature of the currently selected channel
 */
void Control::updatePreset() {
   if (selectedChannel == 0) {
      return;
   }
   Channel &channel = channels[selectedChannel-1];
   NonvolatileArray<int, 3> &chSettings = (selectedChannel==1)?ch1Settings:ch2Settings;

   chSettings.set(channel.preset, channel.targetTemperature);
   channel.modified          = false;
}

/**
 * Interrupt handler for over current comparator
 */
void Control::overCurrentHandler() {

   // Disable drive
   ch1DutyCycleCounter.disable();
   ch2DutyCycleCounter.disable();

   // Mark channels as overloaded
   channels[0].state = ch_overload;
   channels[1].state = ch_overload;

   setNeedsRefresh();
}

/**
 * Interrupt handler for mains zero crossing Comparator
 * This uses the timer to schedule the switchHandler().
 * Occurs @100Hz or 120Hz ~ 10ms or 8.3ms
 */
void Control::zeroCrossingHandler() {

   // Counter to initiate screen refresh
   static unsigned count = 0;

   if (count++>=100) {
      count = 0;
      setNeedsRefresh();
   }

   channels[0].currentTemperature = ch1TipTemperature.getTemperature()+ch1ColdJunctionTemperature.getTemperature();
   channels[1].currentTemperature = ch2TipTemperature.getTemperature()+ch2ColdJunctionTemperature.getTemperature();

//   float ch1DutyCy = ch1Pid.callback(channels[0].targetTemperature, channels[0].currentTemperature);
//   float ch2DutyCy = ch2Pid.callback(channels[1].targetTemperature, channels[1].currentTemperature);

//   ch1DutyCycleCounter.setDutyCycle(ch1DutyCy);
//   ch2DutyCycleCounter.setDutyCycle(ch2DutyCy);

   static auto cb = [](){
      This->switchOnHandler();
   };
   PitChannel::oneShotInMicroseconds(cb, POWER_ON_DELAY);
}

/**
 * Timer interrupt handler for turning on heaters.
 * It also uses the timer to schedule the sampleHandler().
 */
void Control::switchOnHandler() {

   // Enable drive to heaters as needed
   if (ch1DutyCycleCounter.isOn()) {
      Ch1Drive::write(true);
   }
   if (ch2DutyCycleCounter.isOn()) {
      Ch2Drive::write(true);
   }

   Ch1ActiveLed::write(ch1DutyCycleCounter.isOn());
   Ch2ActiveLed::write(ch2DutyCycleCounter.isOn());

   static auto cb = [](){
      This->sampleHandler();
   };
   PitChannel::oneShotInMicroseconds(cb, SAMPLE_DELAY);
}

/**
 * Timer interrupt handler for starting ADC sample sequence.
 * It also uses the timer to schedule the switchOffHandler().
 */
void Control::sampleHandler() {

   // Measure channel 1 if idle this cycle
   if (!ch1DutyCycleCounter.isOn()) {
      adcChannelMask |= (1<<Ch1ColdJunctionNtc::CHANNEL);
      adcChannelMask |= (1<<Ch1TipThermocouple::CHANNEL);
   }
   // Measure channel 2 if idle this cycle
   if (!ch2DutyCycleCounter.isOn()) {
      adcChannelMask |= (1<<Ch2ColdJunctionNtc::CHANNEL);
      adcChannelMask |= (1<<Ch2TipThermocouple::CHANNEL);
   }

   // Always do chip temperature
   // This also start the whole sequence of chained conversions
   ChipTemperature::startConversion(AdcInterrupt_Enabled);

   ch1DutyCycleCounter.advance();
   ch2DutyCycleCounter.advance();

   static auto cb = [](){
      This->switchOffHandler();
   };
   PitChannel::oneShotInMicroseconds(cb, POWER_OFF_DELAY);
}

/**
 * Timer interrupt handler for turning off the heaters.
 */
void Control::switchOffHandler() {
   // Disable drive to heaters as needed
   if (!ch1DutyCycleCounter.isOn()) {
      Ch1Drive::write(false);
   }
   if (!ch2DutyCycleCounter.isOn()) {
      Ch2Drive::write(false);
   }
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
void Control::adcHandler(uint32_t result, int channel) {

   float voltage = result*(ADC_REF_VOLTAGE/Adc0::getSingleEndedMaximum(ADC_RESOLUTION));

   GpioSpare2::set();

   switch (channel) {
      case Ch1ColdJunctionNtc::CHANNEL :
         ch1ColdJunctionTemperature.accumulate(voltage);
         break;
      case Ch1TipThermocouple::CHANNEL :
         ch1TipTemperature.accumulate(voltage);
         break;
      case Ch2ColdJunctionNtc::CHANNEL :
         ch2ColdJunctionTemperature.accumulate(voltage);
         break;
      case Ch2TipThermocouple::CHANNEL :
         ch2TipTemperature.accumulate(voltage);
         break;
      case ChipTemperature::CHANNEL :
         chipTemperature.accumulate(voltage);
         break;
      default:
         // Should be impossible
         __BKPT(0);
   }

   // Set up next conversion
   int channelNum = __builtin_ffs(adcChannelMask)-1;
   if (channelNum >= 0) {
      TemperatureAdc::startConversion(AdcInterrupt_Enabled|channelNum);
   }
   // Indicate done current conversion
   // Note - this means each conversion is done twice except for chip temp.
   adcChannelMask &= ~(1<<channel);

   GpioSpare2::clear();
}

/**
 * Refresh the display of channel information
 */
void Control::refresh() {
   needRefresh = false;

   // Update LEDs
   switch (selectedChannel) {
      case 0:
         Ch1SelectedLed::off();
         Ch2SelectedLed::off();
         break;
      case 1:
         Ch1SelectedLed::on();
         Ch2SelectedLed::off();
         break;
      case 2:
         Ch1SelectedLed::off();
         Ch2SelectedLed::on();
         break;
   }

   // Update display
   display.displayTools(channels[0], channels[1], selectedChannel);
}

/**
 * Debugging code
 */
void Control::reportChannel() {
   static int count=0;
   if (count++ == 2000) {
      count = 0;
      console.setFloatFormat(2, Padding_LeadingSpaces, 2);
      float tipV   = 1000*ch1TipTemperature.getVoltage();
      float tipT   = ch1TipTemperature.getTemperature();
      float coldT  = ch1ColdJunctionTemperature.getTemperature();

      console.
      write("Ch1 Tip = ").write(tipT+coldT).
      write("(").write(tipT).write("+").write(+coldT).
      write("),(").write(tipV).write(" mV").
      write(",").write(ch1ColdJunctionTemperature.getResistance()).write(" ohms)").
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
 * Event loop for front panel events
 */
void Control::eventLoop()  {
   refresh();

   for(;;) {
      reportChannel();

      Event event = switchPolling.getEvent();

      if (event.type != ev_None) {
         // Assume visible change
         setNeedsRefresh();

//         console.write("Position = ").write(event.change).write(", Event = ").writeln(getEventName(event));
         switch(event.type) {
            case ev_Ch1Hold      : toggleEnable(1);             break;
            case ev_Ch2Hold      : toggleEnable(2);             break;
            case ev_Ch1Ch2Hold   :
               if (!isEnabled(1) && !isEnabled(2)) {
                  // Both currently off - turn on both channels
                  enable(2);
                  enable(1); // Active channel
               }
               else {
                  // >= 1 channel on - turn off both
                  disable(1);
                  disable(2);
               }
               break;
            case ev_Tool1Active   : wakeUp(1);                 break;
            case ev_Tool2Active   : wakeUp(2);                 break;
            case ev_Tool1Idle     : backOff(1);                break;
            case ev_Tool2Idle     : backOff(2);                break;
            case ev_Tool1LongIdle : disable(1);                break;
            case ev_Tool2LongIdle : disable(2);                break;
            case ev_Ch1Press      : setSelectedChannel(1);     break;
            case ev_Ch2Press      : setSelectedChannel(2);     break;
            case ev_SelPress      : nextPreset();              break;
            case ev_QuadPress     : updatePreset();            break;
            case ev_QuadRotate    : changeTemp(event.change);  break;
            default: break;
         }
      }
      if (needRefresh) {
         refresh();
      }
      Smc::enterWaitMode();
   }
}

/// this pointer for static members (call-backs)
Control *Control::This = nullptr;

//void Control::testMenu() {
//   for(;;) {
//      display.displayTimeMenuItem(1, "Setback Time", rand()%1000);
//      waitMS(1000);
//      display.displayTemperatureMenuItem(2, "Setback Temp", 100+rand()%400);
//      waitMS(1000);
//   }
//}

