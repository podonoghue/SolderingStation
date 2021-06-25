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

/// Information about tips
Tips tips;

// Create non-volatile storage in FlexRAM
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
