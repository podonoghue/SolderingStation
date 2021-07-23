/*
 *
 */
#include "hardware.h"
#include "Peripherals.h"
#include "Display.h"
#include "SwitchPolling.h"
#include "Channels.h"
#include "Control.h"
#include "pmc.h"
#include "rcm.h"

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
      Ch1Drive::setIn();
      Ch2Drive::setIn();

      __asm__("bkpt");
   }
}
}

int main() {
   initialise();

//   float v[] = {
//         5.758,
//         6,
//         7.546,
//         8,
//         8.974,
//         9
//   };
//   for (unsigned index=0; index<(sizeof(v)/sizeof(v[0])); index++) {
//      console.write(v[index]).write("=>").writeln(ThermocoupleT12Average::convertAdcVoltageToCelsius(v[index]/1000.0/TC_MEASUREMENT_RATIO));
//   }

//   Ch1Drive::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Fast);
//   for(;;) {
//      Ch1Drive::on();
//      waitMS(10);
//      Ch1Drive::off();
//      waitMS(1000);
//   }
   control.eventLoop();

   return 0;
}
