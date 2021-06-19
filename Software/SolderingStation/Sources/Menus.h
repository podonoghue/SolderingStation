/*
 * Menus.h
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_MENUS_H_
#define SOURCES_MENUS_H_
#include "SwitchPolling.h"
#include "SettingsData.h"
#include "TipSettings.h"
#include "Channel.h"

class Menus {
private:
   Menus(const Menus &other) = delete;
   Menus(Menus &&other) = delete;
   Menus& operator=(const Menus &other) = delete;
   Menus& operator=(Menus &&other) = delete;
   Menus() = delete;
   ~Menus() = delete;

public:
   /**
    * Confirm action from user
    *
    * @param prompt Prompt to display
    *
    * @return True if confirmed
    * @return false if cancelled
    */
   static bool confirmAction(const char *prompt);

   /**
    * Display and execute top-level menu
    */
   static void settingsMenu();

   /**
    * Edit a non-volatile time setting.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType editTime(const SettingsData &data);

   /**
    * Edit a non-volatile floating point setting.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType editFloat(const SettingsData &data);

   /**
    * Edit a non-volatile temperatire setting.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType editTemp(const SettingsData &data);

   /**
    * Edit a non-volatile tip setting.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType editTipSettings(const SettingsData &data);

   /**
    * Edit a available tips in non-volatile settings.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType selectAvailableTips(const SettingsData &);

   static bool calibrate(Channel &ch, TipSettings &tipsettings, TipSettings::Calib stage);
};

#endif /* SOURCES_MENUS_H_ */
