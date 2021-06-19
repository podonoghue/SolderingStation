/*
 * Tips.h
 *
 *  Created on: 19 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_TIPS_H_
#define SOURCES_TIPS_H_

#include "TipSettings.h"

class Tips {

public:
   using TipSettingsArray = TipSettings[TipSettings::NUM_TIP_SETTINGS];

   /// Index into tip settings array
   using TipSettingsIndex         = uint8_t;

private:
   Tips(const Tips &other) = delete;
   Tips(Tips &&other) = delete;
   Tips& operator=(const Tips &other) = delete;
   Tips& operator=(Tips &&other) = delete;

   /// Calibration data for tips
   TipSettingsArray  &tipSettings;

public:
   Tips();
   ~Tips() {}

   /**
    * Get tip settings for given index
    *
    * @param index Tip settings index
    *
    * @return Settings describing this tip
    */
   const TipSettings &getTip(TipSettingsIndex tipSettingsIndex) const {
      usbdm_assert(tipSettingsIndex<TipSettings::NUM_TIP_SETTINGS, "Illegal tip index");
      return tipSettings[tipSettingsIndex];
   }

   /**
    * Name of tip for given index
    *
    * @param index Tip settings index
    *
    * @return
    */
   const char *getTipName(TipSettingsIndex tipSettingsIndex) const {
      if (tipSettingsIndex >= TipSettings::NUM_TIP_SETTINGS) {
         return "----";
      }
      return TipSettings::getTipName(tipSettings[tipSettingsIndex].getTipNameIndex());
   }

   /**
    * Locate free tip settings entry
    *
    * @return Pointer to free entry, nullptr if none available.
    */
   TipSettings *findFreeTipSettings() {
      for (unsigned index=0; index<(sizeof(tipSettings)/sizeof(tipSettings[0])); index++) {
         if (tipSettings[index].isFree()) {
            return tipSettings+index;
         }
      }
      return nullptr;
   }

   /**
    * Find an existing tip settings entry by tip name index
    *
    * @return Pointer to existing entry or nullptr if not found
    */
   TipSettings *findTipSettings(TipSettings::TipNameIndex tipNameIndex) {
      for (unsigned index=0; index<(sizeof(tipSettings)/sizeof(tipSettings[0])); index++) {
         if (tipSettings[index].getTipNameIndex() == tipNameIndex) {
            return tipSettings+index;
         }
      }
      return nullptr;
   }

   /**
    * Find an existing tip settings entry by tip name
    *
    * @return Pointer to existing entry or nullptr if not found
    */
   TipSettings *findTipSettings(const char *tipName) {
      return findTipSettings(TipSettings::getTipNameIndex(tipName));
   }

   /**
    * Find or allocate tip settings entry
    *
    * @return Pointer to existing or new entry, nullptr if no space available.
    */
   TipSettings *findOrAllocateTipSettings(TipSettings::TipNameIndex tipNameIndex) {

      TipSettings *tipSettings = findTipSettings(tipNameIndex);
      if (tipSettings == nullptr) {
         tipSettings = findFreeTipSettings();
         if (tipSettings != nullptr) {
            tipSettings->initialise(tipNameIndex);
         }
      }
      return tipSettings;
   }

   /**
    * Find or allocate tip settings entry
    *
    * @return Pointer to existing or new entry, nullptr if no space available.
    */
   TipSettings *findOrAllocateTipSettings(const char *tipName) {
      return findOrAllocateTipSettings(TipSettings::getTipNameIndex(tipName));
   }

};

extern Tips tips;

#endif /* SOURCES_TIPS_H_ */
