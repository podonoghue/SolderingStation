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
