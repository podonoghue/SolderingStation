/*
 * Tips.cpp
 *
 *  Created on: 19 Jun. 2021
 *      Author: peter
 */

#include "Tips.h"
#include "NonvolatileSettings.h"
#include "Display.h"
#include "Channel.h"

// Tip settings used for "NO_TIP"
const TipSettings Tips::NoTipSettings(TipSettings::NO_TIP);

Tips::Tips() : tipSettings(nvinit.tipSettings) {
}

/**
 * Fill menu array with tips currently selected.
 *
 * The array is sorted.\n
 * The array contains pointers to non-volatile data so unnecessary modification should be avoided to reduce EEPROM wear.\n
 * Menu items are marked with a star if checkModifier() evaluates true
 *
 * @param[inout]  menuItems      Array to populate with data
 * @param[in]     checkModifier  Class function to check for set attributes
 */
int Tips::populateSelectedTips(
      MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS],
      bool (TipSettings::*checkModifier)() const) {

   int tipsAllocated = 0;

   // Load tip information for menu from nv-storage
   for(unsigned index=0; index<TipSettings::NUM_TIP_SETTINGS; index++) {
      TipSettings       *ts = getTip(index);
      if (!ts->isFree()) {
         MenuItem &mi = menuItems[tipsAllocated++];
         mi.name          = ts->getTipName();
         mi.nvTipSettings = ts;
         if ((checkModifier != nullptr) && (ts->*checkModifier)()) {
            // Mark calibrated tips with star
            mi.modifiers |= MenuItem::Starred;
         }
      }
   }
   // Sort the menu
   MenuItem::sort(menuItems, tipsAllocated);

   return tipsAllocated;
}

/**
 * Find index of tip in menuItems
 *
 * @param tip           Tip to look for
 * @param menuItems     Array of menu items to search
 * @param tipsAllocated Size of menu items array
 *
 * @return Index within menuItems of the tip currently selected in the channel.
 */
int Tips::findTipInMenu(const TipSettings *tip, MenuItem menuItems[], int tipsAllocated) {

   for (int tipIndex=0; tipIndex<tipsAllocated; tipIndex++) {
      if (menuItems[tipIndex].nvTipSettings == tip) {
         return tipIndex;
      }
   }
   return 0;
}

/**
 * Fill menu array with all tips available.
 *
 * The array is sorted.
 * "NO_TIP" is excluded.
 *
 * @param [inout] tipMenuItems      Array to populate with data
 */
void Tips::populateTips(MenuItem (&tipMenuItems)[TipSettings::NUMBER_OF_VALID_TIPS]) {

   // Copy tip information to menu settings
   unsigned foundTips = 0;
   for(TipSettings::TipNameIndex index=TipSettings::FIRST_VALID_TIP; index<=TipSettings::LAST_VALID_TIP; TipSettings::inc(index), foundTips++) {
      tipMenuItems[foundTips].name          = TipSettings::initialTipInfo[index].name;
      tipMenuItems[foundTips].modifiers     = 0;
      tipMenuItems[foundTips].nvTipSettings = nullptr;

      TipSettings *tip = tips.findTipSettings(index);
      if (tip != nullptr) {
         // Non-volatile tip settings have already been allocated
         tipMenuItems[foundTips].modifiers      |= MenuItem::CheckBoxSelected;
         tipMenuItems[foundTips].nvTipSettings   = tip;
      }
   }

   // Sort the menu
   MenuItem::sort(tipMenuItems, TipSettings::NUMBER_OF_VALID_TIPS);
}
