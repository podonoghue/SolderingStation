/*
 * ChannelSettings.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_CHANNELSETTINGS_H_
#define SOURCES_CHANNELSETTINGS_H_

#include "flash.h"

class ChannelSettings {

public:
   /// Preset temperatures
   USBDM::Nonvolatile<int>    presets[3];

   // Back off temperature for idle iron
   USBDM::Nonvolatile<int>    backOffTemperature;

   // Idle time delay until backing off tip temperature ms
   USBDM::Nonvolatile<int>    backOffTime;

   // Idle time delay until turning off idle iron ms
   USBDM::Nonvolatile<int>    safetyOffTime;
};

#endif /* SOURCES_CHANNELSETTINGS_H_ */
