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
#include "BoundedInteger.h"

class MenuItem;

class Tips {

public:
   /// Type of array for tip settings
   using TipSettingsArray = TipSettings[TipSettings::NUM_TIP_SETTINGS];

   /// Index into tip settings array
   using TipSettingsIndex         = uint8_t;

   static constexpr TipSettingsIndex INVALID_TIP_INDEX = (TipSettingsIndex)-1;

private:
   Tips(const Tips &other) = delete;
   Tips(Tips &&other) = delete;
   Tips& operator=(const Tips &other) = delete;
   Tips& operator=(Tips &&other) = delete;

   /// Nonvolatile arrays of calibration data for available tips as selected by user
   TipSettingsArray  &tipSettings;

   /// Tip settings used for "NO_TIP"
   static const TipSettings NoTipSettings;

public:
   Tips();
   ~Tips() {}

   /**
    * Get default tip (NO_TIP)
    *
    * @return Pointer to tip
    */
   static TipSettings *getDefaultTip() {
      return const_cast<TipSettings *>(&NoTipSettings);
   }

   /**
    * Initialise all Tip non-volatile settings.
    * A default set of tips are loaded.
    */
   void initialiseTipSettings() {
      static const char * const defaultTips[] = {
            "B0",
            "B1",
            "WT50S",
            "WT50M",
            "WT50L",
      };
      for (unsigned index=0; index<(sizeof(defaultTips)/sizeof(defaultTips[0])); index++) {
         TipSettings::TipNameIndex tipIndex = TipSettings::getTipNameIndex(defaultTips[index]);
         tipSettings[index].loadDefaultCalibration(tipIndex);
      }
   }

   /**
    * Get default tip settings for given iron type
    *
    * @param ironType Iron type to look for
    *
    * @return Settings describing this tip or nullptr if none available
    */
   TipSettings *getAvailableTipForIron(IronType ironType) {
      if (ironType == IronType_Unknown) {
         return const_cast<TipSettings *>(&NoTipSettings);
      }
      for (unsigned index=0; index<TipSettings::NUM_TIP_SETTINGS; index++) {
         TipSettings *ts = &tipSettings[index];
         if (ts->isFree()) {
            continue;
         }
         if (ts->getIronType() == ironType) {
            return ts;
         }
      }
      // Try to allocate a suitable tip
      TipSettings *tipSettings = findFreeTipSettings();
      if (tipSettings != nullptr) {
         TipSettings::TipNameIndex tipNameIndex = TipSettings::getDefaultTipForIron(ironType);
         if (tipNameIndex != TipSettings::FREE_ENTRY) {
            tipSettings->loadDefaultCalibration(tipNameIndex);
         }
      }
      return tipSettings;
   }

   /**
    * Get tip settings for given index
    *
    * @param tipSettingsIndex Tip settings index
    *
    * @return Settings describing this tip
    */
   TipSettings *getTip(TipSettingsIndex tipSettingsIndex) const {
      usbdm_assert(tipSettingsIndex<TipSettings::NUM_TIP_SETTINGS, "Illegal tip index");
      return &tipSettings[tipSettingsIndex];
   }

   /**
    * Get next available tip based on current tip
    *
    * @param selectedTip   Current tip
    * @param delta         Offset to wanted tip
    *
    * @return  newly selected tip
    */
   const TipSettings *changeTip(const TipSettings *selectedTip, int delta) {

      // Get sorted list of tips available
      MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS];
      int availableTips = populateSelectedTips(menuItems, nullptr);

      // Get direction to move
      int direction = (delta>0)?1:-1;

      // Can't move further than size of array
      delta %= availableTips;
      delta = abs(delta);

      // Locate existing tip
      int initialIndex = findTipInMenu(selectedTip, menuItems, availableTips);
      CircularInteger index{0, availableTips-1, initialIndex};
      IronType ironType = selectedTip->getIronType();

      do {
         index += direction;
         if (index == initialIndex) {
            // Wrapped around
            break;
         }
         if (menuItems[index].constTipSettings->getIronType() == ironType) {
            delta--;
            if (delta == 0) {
               break;
            }
         }
      } while (true);
      return menuItems[index].constTipSettings;
   }

   /**
    * Name of tip for given index
    *
    * @param tipSettingsIndex Tip settings index
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
   TipSettings *findFreeTipSettings() const {
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
      if (tipNameIndex == TipSettings::NO_TIP) {
         return const_cast<TipSettings *>(&NoTipSettings);
      }
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
            tipSettings->loadDefaultCalibration(tipNameIndex);
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
    * The array contains pointers to non-volatile data so unnecessary modification should be avoided to reduce EEPROM wear
    * Menu items are marked with a star if checkModifier() evaluates true
    *
    * @param[inout]  menuItems      Array to populate with data
    * @param[in]     checkModifier  Class function to check for set attributes
    */
   int populateSelectedTips(
         MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS],
         bool (TipSettings::*checkModifier)() const);

   /**
    * Find index of tip in menuItems
    *
    * @param tip           Tip to look for
    * @param menuItems     Array of menu items to search
    * @param tipsAllocated Size of menu items array
    *
    * @return Index within menuItems of the tip currently selected in the channel.
    */
   static int findTipInMenu(const TipSettings *tip, MenuItem menuItems[], int tipsAllocated);

   /**
    * Fill menu array with all tips available.
    *
    * The array is sorted.
    * The "NO_TIP" is not included.
    *
    * @param [inout] tipMenuItems      Array to populate with data
    */
   void populateTips(MenuItem (&tipMenuItems)[TipSettings::NUMBER_OF_VALID_TIPS]);

};

extern Tips tips;

#endif /* SOURCES_TIPS_H_ */
