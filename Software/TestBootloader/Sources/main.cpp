/*
 ============================================================================
 * @file    main.cpp (180.ARM_Peripherals/Sources/main.cpp)
 * @brief   Basic C++ demo
 *
 *  Created on: 10/1/2016
 *      Author: podonoghue
 ============================================================================
 */
#include "hardware.h"
#include "BootloaderInfo.h"
#include "stringFormatter.h"
#include "Peripherals.h"

// Allow access to USBDM methods without USBDM:: prefix
using namespace USBDM;

/**
 * See more examples in Snippets directory
 */

static constexpr unsigned HARDWARE_VERSION = HW_SOLDER_STATION_V3;

static uint32_t magicNumber = 0;

__attribute__((used))
__attribute__ ((section(".bootloader"))) extern
BootInformation const bootInformation = {
      &magicNumber,        // Pointer to magic number location to force ICP
      1,                   // Version of this software image
      HARDWARE_VERSION,    // Identifies the hardware this image is intended for
      0,                   // Filled in by bootloader
};

/**
 * Reset system to bootloader mode
 */
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


using Led = GpioC<6, ActiveHigh>;

int main() {
   // Power-on message
   StringFormatter_T<40> sf;
   sf.write("SW:V").writeln(bootInformation.softwareVersion)
     .write("HW:").writeln(getHardwareType<HARDWARE_VERSION>());
   console.write(sf.toString());

   Led::setOutput(PinDriveStrength_High, PinDriveMode_PushPull, PinSlewRate_Slow);

   for(int count = 0;count < 20;count++) {
      Led::toggle();
      waitMS(100);
      console.write(count).writeln(": Tick...");
   }

   // Back to bootloader
   console.writeln("Back to bootloader");
   resetToBootloader();

   return 0;
}
