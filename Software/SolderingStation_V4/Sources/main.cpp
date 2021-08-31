/*
 *
 */
#include "hardware.h"
#include "BootloaderInfo.h"
#include "stringFormatter.h"
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
      // Disable drive on crash!
      Ch1Drive<0>::Owner::setIn();
      Ch2Drive<0>::Owner::setIn();

      __asm__("bkpt");
   }
}
}

static constexpr unsigned HARDWARE_VERSION = HW_SOLDER_STATION_V3;

static uint32_t magicNumber = 0;

__attribute__((used))
#ifdef RELEASE_BUILD
// Make bootloader information visible to linker
// Triggers changes to memory map to suit bootloader
__attribute__ ((section(".bootloader"))) extern
#else
// No bootloader used
static
#endif
BootInformation const bootInformation = {
      &magicNumber,        // Pointer to magic number location to force ICP
      1,                   // Version of this software image
      HARDWARE_VERSION,    // Identifies the hardware this image is intended for
      0,                   // Filled in by bootloader
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
   *bootInformation.magicNumber = MAGIC_NUMBER;

   // Request system reset
   SCB->AIRCR = SCB_AIRCR_VECTKEY(0x5FA) | SCB_AIRCR_SYSRESETREQ_Msk;

   // Wait until reset
   for(;;) {
      __asm__("nop");
   }
}

int main() {
   // Power-on message
   StringFormatter_T<40> sf;
   sf.write("SW:V").writeln(bootInformation.softwareVersion)
     .write("HW:").writeln(getHardwareType<HARDWARE_VERSION>());
   display.showMessage("Starting", sf.toString());
   waitMS(5000);

//   using Drive = Ch1Drive<0>::Owner;
//   Drive::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Fast);
//   for(;;) {
//      Drive::write(0b11);
//      waitMS(10);
//      Drive::write(0b00);
//      waitMS(1000);
//   }
   control.eventLoop();

   resetToBootloader();

   return 0;
}
