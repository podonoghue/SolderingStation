/*
 ============================================================================
 * @file    main.cpp
 * @brief   Main-line for Soldering station
 *
 *  Created on: 10/1/2016
 *      Author: podonoghue
 ============================================================================
 */
#include "hardware.h"
#include "stringFormatter.h"
#include "Display.h"
#include "SwitchPolling.h"
#include "Channels.h"
#include "Control.h"
#include "rcm.h"
#include "NonvolatileSettings.h"
#include "BootInformation.h"

using namespace USBDM;

/// Information about tips
Tips tips;

/// Create non-volatile storage in FlexRAM
__attribute__ ((section(".flexRAM")))
NonvolatileSettings nvinit;

/// Channel interface and state
Channels       channels;

/// Polls the switches and the set-back switches
SwitchPolling  switchPolling;

/// Main control class
Control        control;

/// Handles the OLED display
Display        display;

void initialise() {
   // Turn on filtering of reset pin
   Rcm::configure(RcmResetPinRunWaitFilter_LowPowerOscillator, RcmResetPinStopFilter_LowPowerOscillator);

   display.initialise();
   control.initialise();
   switchPolling.initialise();
}

extern "C" {
/**
 * exit
 *
 * Exit process - overridden to ensure heaters are off
 *
 * @param rc - Return code from process
 */
void _exit(int rc __attribute__((unused))) {
   for(;;) {
      // Disable drive on crash!
      ch1Drive.disablePins();
      ch2Drive.disablePins();
      __asm__("bkpt");
   }
}
}

static constexpr HardwareType HARDWARE_VERSION = HW_SOLDER_STATION_V4;

__attribute__ ((section(".noinit")))
static uint32_t magicNumber;

#if defined(RELEASE_BUILD)
// Triggers memory image relocation for bootloader
extern BootInformation const bootloaderInformation;
#endif

__attribute__ ((section(".bootloaderInformation")))
__attribute__((used))
const BootInformation bootloaderInformation = {
      &magicNumber,        // Magic number to force ICP on reboot
      1,                   // Version of this software image
      HARDWARE_VERSION,    // Hardware version for this image
};

/**
 * Reset system to bootloader mode
 */
__attribute__((unused))
static void resetToBootloader() {

#ifndef SCB_AIRCR_VECTKEY
#define SCB_AIRCR_VECTKEY(x) (((x)<<SCB_AIRCR_VECTKEY_Pos)&SCB_AIRCR_VECTKEY_Msk)
#endif

   // Set ICP on reboot
   magicNumber = MAGIC_NUMBER;

   // Request system reset
   SCB->AIRCR = SCB_AIRCR_VECTKEY(0x5FA) | SCB_AIRCR_SYSRESETREQ_Msk;

   // Wait until reset
   for(;;) {
      __asm__("nop");
   }
}

int main() {

   console.writeln("Reset Source = ", Rcm::getResetSourceDescription());
   if (Rcm::getResetSource() & RcmSource_Wdog) {
      console.writeln("Watchdog reset - halting");
      for(;;) {
         __BKPT();
      }
   }

   // Power-on message
   StringFormatter_T<40> sf;
   sf.write("SW:V").writeln(bootloaderInformation.softwareVersion)
     .write("HW:").writeln(getHardwareType(HARDWARE_VERSION));
   display.showMessage("Starting", sf.toString());
   waitMS(2000);

#if 0
   ch1Drive.setOutput();
   ch2Drive.setOutput();

   analogueMultiplexor.setOutput();

   bool toggle = false;

   for(;;) {
      display.enable(toggle);
      toggle = !toggle;
      analogueMultiplexor.write(MuxSelect_Ch1aHighGain);
      analogueMultiplexor.write(MuxSelect_Ch1aHighGainBoost);
      analogueMultiplexor.write(MuxSelect_Ch1aHighGainBiased);
      analogueMultiplexor.write(MuxSelect_Ch1aHighGainBoostBiased);
   }
#endif
#if 0
   ch1Drive.setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Fast);
   for(;;) {
      ch1Drive.write(0b10);
      waitMS(100);
      ch1Drive.write(0b01);
      waitMS(100);
      ch1Drive.write(0b00);
      waitMS(1000);
   }
#endif
#if defined(DEBUG_BUILD)
//   console.writeln(Rcm::getResetSourceDescription());
   console.writeln("nvinit.hardwareCalibration.preAmplifierNoBoost   = ", 1/(float)nvinit.hardwareCalibration.preAmplifierNoBoost);
   console.writeln("nvinit.hardwareCalibration.preAmplifierWithBoost = ", 1/(float)nvinit.hardwareCalibration.preAmplifierWithBoost);
   console.writeln("BIAS_RESISTOR_VALUE                   = ", BIAS_RESISTOR_VALUE);
   console.writeln("BIAS_VOLTAGE                          = ", BIAS_VOLTAGE);
   console.writeln("ADC_REF_VOLTAGE                       = ", ADC_REF_VOLTAGE);
   console.writeln("LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF  = ", LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF);
   console.writeln("getSingleEndedMaximum(ADC_RESOLUTION) = ", USBDM::FixedGainAdc::getSingleEndedMaximum(ADC_RESOLUTION));
   console.writeln("IdentifyMeasurementRatio              = ", Channel::IdentifyMeasurementRatio);

#endif

   initialise();

   control.eventLoop();

   resetToBootloader();

   return 0;
}
