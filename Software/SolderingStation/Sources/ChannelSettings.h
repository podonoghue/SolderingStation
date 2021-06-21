/*
 * ChannelSettings.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_CHANNELSETTINGS_H_
#define SOURCES_CHANNELSETTINGS_H_

#include "flash.h"
#include "Tips.h"

class ChannelSettings {

public:
   /// Preset temperatures C
   USBDM::Nonvolatile<uint16_t> presets[3];

   /// Set-back temperature for idle iron C
   USBDM::Nonvolatile<uint16_t> setbackTemperature;

   /// Idle time delay until reducing tip temperature in seconds
   USBDM::Nonvolatile<uint16_t> setbackTime;

   /// Idle time delay until turning off iron in seconds
   USBDM::Nonvolatile<uint16_t> safetyOffTime;

   /// Selected tip for channel
   USBDM::Nonvolatile<const TipSettings *> selectedTip;

   void initialise() {
      static constexpr unsigned DEFAULT_IDLE_TIME      =  5*60; //  5 minutes in seconds
      static constexpr unsigned DEFAULT_LONGIDLE_TIME  = 20*60; // 20 minutes in seconds

      presets[0]           = 250;
      presets[1]           = 350;
      presets[2]           = 370;
      setbackTemperature   = 200;
      setbackTime          = DEFAULT_IDLE_TIME;
      safetyOffTime        = DEFAULT_LONGIDLE_TIME;
      selectedTip          = tips.getTip(0);
   }
};

#endif /* SOURCES_CHANNELSETTINGS_H_ */
