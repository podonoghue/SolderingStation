/*
 * SettingsData.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_SETTINGSDATA_H_
#define SOURCES_SETTINGSDATA_H_

#include "flash.h"

/**
 * Data describing how to edit a non-volatile setting
 */
class SettingsData {

public:
   enum Type {Temperature, Time, Pid, Tip};

   const char      * const name;
   EventType (*handler)(const SettingsData &);

   union {
      USBDM::Nonvolatile<uint16_t>  *settingUint16;
      USBDM::Nonvolatile<int>       *settingInt;
      USBDM::Nonvolatile<float>     *settingFloat;
   };
   union {
      int       increment;
      int       option;
   };

   /**
    * Constructor for Nonvolatile<int> setting
    *
    * @param name       Name to display
    * @param handler    Code to handle changes
    * @param setting    The non-volatile value being modified
    * @param increment  How large an increment for rotary encoder indent
    */
   constexpr SettingsData(const char *name, EventType (*handler)(const SettingsData &), USBDM::Nonvolatile<int> &setting, int increment)
   : name(name), handler(handler), settingInt(&setting), increment(increment) {
   }

   /**
    * Constructor for Nonvolatile<uint16_t> setting
    *
    * @param name       Name to display
    * @param handler    Code to handle changes
    * @param setting    The non-volatile value being modified
    * @param increment  How large an increment for rotary encoder indent
    */
   constexpr SettingsData(const char *name, EventType (*handler)(const SettingsData &), USBDM::Nonvolatile<uint16_t> &setting, int increment)
   : name(name), handler(handler), settingUint16(&setting), increment(increment) {
   }

   /**
    * Constructor for Nonvolatile<float> setting
    *
    * @param name       Name to display
    * @param handler    Code to handle changes
    * @param setting    The non-volatile value being modified
    * @param increment  How large an increment for rotary encoder indent
    */
   constexpr SettingsData(const char *name, EventType (*handler)(const SettingsData &), USBDM::Nonvolatile<float> &setting, int increment)
   : name(name), handler(handler), settingFloat(&setting), increment(increment) {
   }

   /**
    * Constructor for general non-volatile setting
    *
    * @param name       Name to display
    * @param handler    Code to handle changes
    */
   constexpr SettingsData(const char *name, EventType (*handler)(const SettingsData &))
   : name(name), handler(handler), settingFloat(nullptr), increment(0) {
   }
   /**
    * Constructor for general non-volatile setting
    *
    * @param name       Name to display
    * @param handler    Code to handle changes
    */
   constexpr SettingsData(const char *name, EventType (*handler)(const SettingsData &), int option)
   : name(name), handler(handler), settingFloat(nullptr), option(option) {
   }
};

#endif /* SOURCES_SETTINGSDATA_H_ */
