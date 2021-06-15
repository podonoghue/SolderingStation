/*
 * NonvolatileSettings.cpp
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#include "NonvolatileSettings.h"

// Create non-volatile storage in FlexRAM
__attribute__ ((section(".flexRAM")))
NonvolatileSettings nvinit;

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

void NonvolatileSettings::initialiseChannelSettings(ChannelSettings &settings) {

   constexpr unsigned IDLE_MAX_TIME      =  5*60*1000; //  5 minutes in ms
   constexpr unsigned LONGIDLE_MAX_TIME  = 20*60*1000; // 20 minutes in ms

   settings.presets[0]           = 250;
   settings.presets[1]           = 350;
   settings.presets[2]           = 370;
   settings.setbackTemperature   = 200;
   settings.setbackTime          = IDLE_MAX_TIME;
   settings.safetyOffTime        = LONGIDLE_MAX_TIME;
}

void NonvolatileSettings::initialisePidSettings() {
   pidSettings.kp = 5;
   pidSettings.ki = 0;
   pidSettings.kd = 0;
}

void NonvolatileSettings::initialiseNonvolatileStorage() {
   initialiseChannelSettings(ch1Settings);
   initialiseChannelSettings(ch2Settings);
   initialisePidSettings();
}


