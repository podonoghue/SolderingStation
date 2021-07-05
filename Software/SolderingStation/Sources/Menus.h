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
    * @return False if cancelled
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
    * Edit a non-volatile temperature setting.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType editTemperature(const SettingsData &data);

   /**
    * Calibrate a tip
    *
    * @param[in]     ch            Channel being used
    * @param[in/out] tipsettings   Tipsettings being determined
    * @param[in]     stage         Stage is calibration sequence
    *
    * @return  false - abort or failed
    * @return  true  - continue to next stage
    */
   static bool calibrateTipTemp(Channel &ch, TipSettings &tipsettings, TipSettings::Calib stage);

   /**
    * Calculate a non-volatile tip calibration setting.
    *
    * @param data Data describing setting to change
    *
    * @return Exiting event
    */
   static EventType calibrateTipTemps(const SettingsData &data);

   /**
    * Edit a non-volatile tip calibration setting.
    *
    * @param settings Data describing setting to change
    *
    * @return Exiting event
    */
   static bool editPidSetting(TipSettings &tipSettings);

   /**
    * Edit a non-volatile tip PID setting.
    *
    * @return Exiting event
    */
   static EventType editPidSettings(const SettingsData &);

   /**
    * Calculate a non-volatile tip PID settings.
    *
    * @return Exiting event
    */
   static EventType stepResponse(const SettingsData &);

   /**
    * Edit a available tips in non-volatile settings.
    *
    * @return Exiting event
    */
   static EventType selectAvailableTips(const SettingsData &);

   static EventType displayChannelStatuses(const SettingsData &);
};

#endif /* SOURCES_MENUS_H_ */
