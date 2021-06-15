/*
 * SettingsData.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_SETTINGSDATA_H_
#define SOURCES_SETTINGSDATA_H_

#include "flash.h"

class SettingsData {

public:
   enum Type {Temperature, Time, Pid};

   const char      * const name;
   const Type              type;
   union {
      USBDM::Nonvolatile<int>       *settingInt;
      USBDM::Nonvolatile<float>     *settingFloat;
   };
   int       increment;

   constexpr SettingsData(const char *name, USBDM::Nonvolatile<int> &setting, int increment, Type type)
   : name(name), type(type), settingInt(&setting), increment(increment) {
   }
   constexpr SettingsData(const char *name, USBDM::Nonvolatile<float> &setting, int increment, Type type)
   : name(name), type(type), settingFloat(&setting), increment(increment) {
   }

   void doIncrement(int delta, int &scratch) const {
      static constexpr int MAX_TIME = (99*60+50)*1000; // in milliseconds

//      USBDM::console.writeln(delta);

      switch (type) {
         case Temperature:
            // Increment by multiple of scale with forced rounding
            scratch += delta*increment;
            scratch  = scratch - scratch%increment;

            if (scratch>Control::MAX_TEMP) {
               scratch = Control::MAX_TEMP;
            }
            if (scratch<Control::MIN_TEMP) {
               scratch = Control::MIN_TEMP;
            }
            break;

         case Time:
            // Increment by multiple of scale with forced rounding
            scratch += delta*increment;
            scratch  = scratch - scratch%increment;

            if (scratch>MAX_TIME) {
               scratch = MAX_TIME;
            }
            if (scratch<0) {
               scratch = 0;
            }
            break;

         case Pid:
            scratch += delta*increment;
            scratch  = scratch - scratch%increment;

            if (scratch<0) {
               scratch = 0;
            }
            break;
      }
   }

   int convertFromFloat(float valueIn) const {
      return round(valueIn * 1000);
   }

   int convertFromInt(int valueIn) const {
      return valueIn;
   }

   float convertToFloat(int valueIn) const {
      return valueIn/1000.0;
   }

   int convertToInt(int valueIn) const {
      return valueIn;
   }
};

#endif /* SOURCES_SETTINGSDATA_H_ */
