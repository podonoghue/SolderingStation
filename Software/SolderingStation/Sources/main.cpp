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

using namespace USBDM;

/// Handles the OLED display
Display        display;

/// Polls the switches and the set-back switches
SwitchPolling  switchPolling;

/// Main control class
Control        control;

/// Channel interface and state
Channels       channels;

void initialise() {
   display.initialise();
   control.initialise();
   switchPolling.initialise();
}

int main() {

   console.WRITELN("Starting\n");

   initialise();

//   control.testMenu();

//   reportBandgapVoltage();
//      reportChipTemperature();
//      testDrive();

      control.eventLoop();

   return 0;
}
