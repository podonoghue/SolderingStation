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

   // Setback temperature for idle iron
   USBDM::Nonvolatile<int>    setbackTemperature;

   // Idle time delay until reducing tip temperature ms
   USBDM::Nonvolatile<int>    setbackTime;

   // Idle time delay until turning off iron ms
   USBDM::Nonvolatile<int>    safetyOffTime;
};

#endif /* SOURCES_CHANNELSETTINGS_H_ */
