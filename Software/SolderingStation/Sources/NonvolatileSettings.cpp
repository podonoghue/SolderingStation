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
   volatile USBDM::FlashDriverError_t rc = initialiseEeprom<EepromSel_2KBytes, PartitionSel_flash0K_eeprom32K, SplitSel_disabled>();

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
 * Initialise a non-volatile channel settings object
 *
 * @param settings Settings for a channel
 */
void NonvolatileSettings::initialiseChannelSettings() {
   ch1Settings.initialise();
   ch2Settings.initialise();
}

/**
 * Initialise all Tip non-volatile settings.
 * All entries except 1st are cleared.
 */
void NonvolatileSettings::initialiseTipSettings() {
   tipSettings[0].setDefaultCalibration();
}

/**
 * Initialise non-volatile storage to default values
 */
void NonvolatileSettings::initialiseNonvolatileStorage() {
   initialiseTipSettings();
   initialiseChannelSettings();
}

