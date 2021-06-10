/*
 *
 */
#include "hardware.h"
#include "Peripherals.h"
#include "Display.h"
#include "SwitchPolling.h"
#include "Channel.h"
#include "Control.h"
#include "pmc.h"

using namespace USBDM;

/// Handles the OLED display
Display        display;

/// Polls the switches and the set-back switches
SwitchPolling  switchPolling;

/// Main control class
Control        control;

/// The channel information
Channels       channels;

void reportChipTemperature() {
   using ChipTemperature = Adc0::Channel<26>;

   Adc0::configure(
         ADC_RESOLUTION,
         AdcClockSource_Bus,
         AdcSample_20,
         AdcPower_Normal,
         AdcMuxsel_B,
         AdcClockRange_Normal,
         AdcAsyncClock_Disabled);
   Adc0::calibrate();
   Adc0::setAveraging(AdcAveraging_32);

   for(;;) {
      unsigned tMeasure        = ChipTemperature::readAnalogue();
      float    tVoltage        = tMeasure*(ADC_REF_VOLTAGE/Adc0::getSingleEndedMaximum(ADC_RESOLUTION));
      // Formula from data sheets
      float    chipTemperature = 25 - (tVoltage-0.719)/.001715;

      if ((chipTemperature<10) || (chipTemperature>35)) {
         __asm__("nop");
      }
      console.setFloatFormat(1, Padding_LeadingSpaces, 2);
      console.write("Temp = ").write(chipTemperature).writeln(" degrees");
      console.resetFormat();
   }
}

void reportBandgapVoltage() {

   Adc0::configure(
         ADC_RESOLUTION,
         AdcClockSource_Bus,
         AdcSample_20,
         AdcPower_Normal,
         AdcMuxsel_B,
         AdcClockRange_Normal);
   Adc0::setAveraging(AdcAveraging_32);

//   using Bandgap    = Adc0::Channel<27>;  // Internal Bandgap voltage reference
   Pmc::configureBandgapOperation(PmcBandgapBuffer_On, PmcBandgapLowPowerEnable_Off);

   for(;;) {
      unsigned tMeasure        = BandGap::readAnalogue();
      float    bandgapVoltage  = tMeasure*(ADC_REF_VOLTAGE/Adc0::getSingleEndedMaximum(ADC_RESOLUTION));

      if ((bandgapVoltage<0.99)||(bandgapVoltage>1.01)) {
         console.write("Opps - ");
      }
      console.setFloatFormat(6, Padding_LeadingSpaces, 2);
      console.write("Bandgap = ").write(bandgapVoltage).writeln(" volts");
      console.resetFormat();
   }
}

void testDrive() {
   Ch1Drive::setOutput(
         PinDriveStrength_High,
         PinDriveMode_PushPull,
         PinSlewRate_Slow);
//   Ch2Drive::setOutput(
//         PinDriveStrength_High,
//         PinDriveMode_PushPull,
//         PinSlewRate_Slow);

   static constexpr int OnTime = 10; // ms 10ms == 1 cycle
   for(;;) {
      Ch1Drive::on();
//      Ch2Drive::on();
      waitMS(OnTime);
      Ch1Drive::off();
//      Ch2Drive::off();
      waitMS(1000-OnTime);
   }
}

void initialise() {
   display.initialise();
   control.initialise();
   switchPolling.initialise();
}

int main() {

   console.writeln("Starting\n");
   console.write("SystemCoreClock = ").writeln(SystemCoreClock);
   console.write("SystemBusClock  = ").writeln(SystemBusClock);

   initialise();

   control.testMenu();

//   reportBandgapVoltage();
//      reportChipTemperature();
//      testDrive();

      control.eventLoop();

   return 0;
}
