/*
 * NonvolatileSettings.cpp
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#include "NonvolatileSettings.h"

/**
 * Constructor
 */
NonvolatileSettings::NonvolatileSettings() : Flash() {

   // Initialise the non-volatile system and configure if necessary
   volatile USBDM::FlashDriverError_t rc = initialiseEeprom<EepromSel_1KBytes, PartitionSel_flash0K_eeprom32K, SplitSel_disabled>();

   if (rc == USBDM::FLASH_ERR_NEW_EEPROM) {
      // This is the first reset after programming the device
      // Initialise the non-volatile variables as necessary
      // If not initialised they will have an initial value of 0xFF

      initialiseNonvolatileStorage();
   }
   else {
      usbdm_assert(rc == USBDM::FLASH_ERR_OK, "FlexNVM initialisation error");
      USBDM::console.WRITELN("Not initialising NV variables");
   }
}

/**
 * Initialise non-volatile storage to default values
 */
void NonvolatileSettings::initialiseNonvolatileStorage() {
   tips.initialiseTipSettings();
   ch1Settings.initialise();
   ch2Settings.initialise();
   hardwareCalibration.initialise();
}

