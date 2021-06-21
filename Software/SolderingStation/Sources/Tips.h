/*
 * Tips.h
 *
 *  Created on: 19 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_TIPS_H_
#define SOURCES_TIPS_H_

#include "TipSettings.h"
#include "Display.h"

class MenuItem;

class Tips {

public:
   /// Type of array for tip settings
   using TipSettingsArray = TipSettings[TipSettings::NUM_TIP_SETTINGS];

   /// Index into tip settings array
   using TipSettingsIndex         = uint8_t;

private:
   Tips(const Tips &other) = delete;
   Tips(Tips &&other) = delete;
   Tips& operator=(const Tips &other) = delete;
   Tips& operator=(Tips &&other) = delete;

   /// Nonvolatile arrays of calibration data for tips
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
   TipSettings *getTip(TipSettingsIndex tipSettingsIndex) const {
      usbdm_assert(tipSettingsIndex<TipSettings::NUM_TIP_SETTINGS, "Illegal tip index");
      return &tipSettings[tipSettingsIndex];
   }

   const TipSettings *getNextTip(const TipSettings *selectedTip) {
      // Get sorted list of tips available
      MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS];
      unsigned availableTips = populateSelectedTips(menuItems, nullptr);

      if (selectedTip == nullptr) {
         return menuItems[0].tipSettings;
      }

      // Locate exiting tip
      unsigned index;
      for(index=0; index<availableTips; index++) {
         TipSettings *ts = menuItems[index].tipSettings;
         if (ts == selectedTip) {
            break;
         }
      }
      index++;
      if (index >= availableTips) {
         index = 0;
      }
      return menuItems[index].tipSettings;
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
            tipSettings->setDefaultCalibration(tipNameIndex);
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

   /**
    * Fill menu array with tips currently selected.
    * The array is sorted.
    * Menu items are marked with a star if checkModifier() evaluates true
    *
    * @param[in/out] menuItems      Array to populate with data
    * @param[in]     checkModifier  Class function to check for set attributes
    */
   unsigned populateSelectedTips(
         MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS],
         bool (TipSettings::*checkModifier)() const);

   /**
    * Fill menu array with all tips available.
    * The array is sorted.
    * Menu items are marked with check-box if a corresponding non-volatile TipSetting exists.
    *
    * @param[in/out] menuItems      Array to populate with data
    */
   void populateTips(MenuItem menuItems[TipSettings::NUMBER_OF_TIPS]);

};

extern Tips tips;

#endif /* SOURCES_TIPS_H_ */
