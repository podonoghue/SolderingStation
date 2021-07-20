/*
 * Menus.cpp
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#include "Menus.h"
#include "BoundedInteger.h"
#include "Display.h"
#include "Control.h"
#include "NonvolatileSettings.h"
#include "stringFormatter.h"
#include "hardware.h"
#include "Channels.h"
#include "StepResponseDriver.h"

using namespace USBDM;

/**
 * Edit a non-volatile time setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::editTime(const SettingsData &data) {
   static constexpr int MAX_TIME = (99*60+50); // in seconds

   Event event;;

   BoundedInteger scratch(0,  MAX_TIME, *data.settingUint16);

   int unchanged = scratch;

   bool menuContinue = true;
   bool doRefresh    = true;
   do {
      if (doRefresh) {
         doRefresh = false;
         display.displayTimeMenuItem(data.name, scratch, scratch != unchanged);
      }
      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }
      switch (event.type) {
         case ev_QuadRelease:
            *data.settingUint16 = scratch;
            unchanged = scratch;
            doRefresh = true;
            break;

         case ev_QuadRotate:
            scratch += event.change * data.increment;
            scratch -= scratch % data.increment;
            doRefresh = true;
            break;

         case ev_QuadRotatePressed:
            scratch += event.change * 6*data.increment;
            scratch -= scratch % data.increment;
            doRefresh = true;
            break;

         case ev_QuadHold:
         case ev_SelRelease:
         case ev_SelHold:
         case ev_Ch1Release:
         case ev_Ch2Release:
            menuContinue = false;
            break;

         default:
            break;
      }
   } while (menuContinue);

   return event.type;
}

/**
 * Edit a non-volatile floating point setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::editFloat(const SettingsData &data) {

   Event event;

   BoundedInteger scratch(0, INT_MAX, round(*data.settingFloat * 1000));

   int unchanged = scratch;

   bool menuContinue = true;
   bool doRefresh    = true;
   do {
      if (doRefresh) {
         doRefresh = false;
         display.displayFloatMenuItem(data.name, scratch, scratch != unchanged);
      }
      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }
      switch (event.type) {
         case ev_QuadRelease:
            *data.settingFloat = scratch/1000.0;
            unchanged = scratch;
            doRefresh = true;
            break;

         case ev_QuadRotate:
            scratch += event.change * data.increment;
            scratch -= scratch % data.increment;
            doRefresh = true;
            break;

         case ev_QuadRotatePressed:
            scratch += event.change * 10 * data.increment;
            scratch -= scratch % data.increment;
            doRefresh = true;
            break;

         case ev_QuadHold:
         case ev_SelRelease:
         case ev_SelHold:
         case ev_Ch1Release:
         case ev_Ch2Release:
            menuContinue = false;
            break;

         default:
            break;
      }
   } while (menuContinue);

   return event.type;
}

/**
 * Edit a non-volatile temperature setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::editTemperature(const SettingsData &data) {

   Event event;;

   BoundedInteger scratch(Control::MIN_TEMP, Control::MAX_TEMP, *data.settingUint16);

   int unchanged = scratch;

   bool menuContinue = true;
   bool doRefresh    = true;
   do {
      if (doRefresh) {
         doRefresh = false;
         display.displayTemperatureMenuItem(data.name, scratch, scratch != unchanged);
      }
      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }
      switch (event.type) {
         case ev_QuadRelease:
            *data.settingUint16 = scratch;
            unchanged = scratch;
            doRefresh = true;
            break;

         case ev_QuadRotate:
            // Increment by multiple of scale with forced rounding
            scratch += event.change * data.increment;
            scratch -= scratch % data.increment;
            doRefresh = true;
            break;

         case ev_QuadRotatePressed:
            // Increment by multiple of scale with forced rounding
            scratch += event.change * 10 * data.increment;
            scratch -= scratch % data.increment;
            doRefresh = true;
            break;

         case ev_QuadHold:
         case ev_SelRelease:
         case ev_SelHold:
         case ev_Ch1Release:
         case ev_Ch2Release:
            menuContinue = false;
            break;

         default:
            break;
      }
   } while (menuContinue);

   return event.type;
}

/**
 * Calibrate a tip
 *
 * @param[in]     ch            Channel being used
 * @param[in/out] tipsettings   Tip settings being determined
 * @param[in]     stage         Stage is calibration sequence
 *
 * @return  false - abort or failed
 * @return  true  - continue to next stage
 */
bool Menus::calibrateTipTemp(Channel &ch, TipSettings &tipsettings, CalibrationIndex stage) {

   if (tipsettings.isFree()) {
      return false;
   }
   //   console.setFloatFormat(0, Padding_LeadingSpaces, 2);
   //   console.write("Test = ").writeln(1.2f);

   bool doUpdate = true;

   // Temperature being calibrated to
   const int targetTemperature = TipSettings::getCalibrationTemperature(stage);

   // User temperature value used to allow temperature adjustment to obtain targetTemperature
   // Start at target-50 as much faster to heat than cool i.e. approach from below is more convenient
   BoundedInteger controlledTemperature{Control::MIN_TEMP, Control::MAX_TEMP, (int)targetTemperature-50};

   // Set up channel for this tip
   ch.setTip(&tipsettings);
   ch.setUserTemperature(controlledTemperature);
   ch.setState(ch_active);

   StringFormatter_T<16> title;

   title.write("Stage ").write(stage+1).write(" - ").write(tipsettings.getTipName());

   enum {working, complete, fail} loopControl = working;

   do {
//      control.reportPid(ch);
      if (doUpdate || control.needsRefresh()) {
         display.displayCalibration(title.toString(), ch, targetTemperature);
      }
      Event event = switchPolling.getEvent();
      switch(event.type) {
         case ev_QuadRotate:
            // Adjust user temperature
            controlledTemperature += event.change;
            ch.setUserTemperature(controlledTemperature);
            doUpdate = true;
            break;

         case ev_SelRelease:
         case ev_QuadRelease:
            // Save calibration point values to tip-settings
            ch.measurement->setCalibrationPoint(stage, tipsettings);
            loopControl = complete;
            break;

         case ev_SelHold:
         case ev_QuadHold:
            // Abort
            loopControl = fail;
            break;

         default:
            break;
      }
   } while(loopControl == working);

   ch.setState(ch_off);
   return loopControl == complete;
}

/**
 * Calculate a non-volatile tip calibration setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::calibrateTipTemps(const SettingsData &) {

   // For debugging - hack to disable channels
//   Ch1Drive::setIn();
//   Ch2Drive::setIn();

   Channel &channel = channels[1];

   static constexpr unsigned modifiers = MenuItem::Starred;

   MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS] = {0};

   int tipsAllocated = tips.populateSelectedTips(menuItems, &TipSettings::isTemperatureCalibrated);

   int  offset    = 0;
   BoundedInteger  selection{0, tipsAllocated-1, Tips::findTipInMenu(channels[1].getTip(), menuItems, tipsAllocated)};

   // This is a dummy settings object that is NOT in nv-storage
   // Made static to avoid warnings about non-initialised object
   static TipSettings tipSettings;

   bool refresh  = true;
   enum {working, complete, fail} loopControl = working;
   Event event;

   do {
      if (refresh) {
         display.displayMenuList("Temp Calibration", menuItems, modifiers, offset, selection);
         refresh = false;
      }

      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }

      // Assume refresh required
      refresh = true;

      switch (event.type) {

         case ev_SelRelease:
         case ev_QuadRelease: {
            if (menuItems[selection].name == nullptr) {
               break;
            }
            Event ev = display.displayMessage(
                  "Calibrate",
                  "Using an external\n"
                  "tip thermometer,\n"
                  "adjust tip temp.\n"
                  "to Target value.\n\n"
                  "Press'n'hold to abort");

            if (ev.isSelRelease()) {
               tipSettings = *menuItems[selection].tipSettings;
//               tipSettings.setTipNameIndex(menuItems[selection].tipSettings->getTipNameIndex());
               bool success =
                     calibrateTipTemp(channel, tipSettings, CalibrationIndex_250) &&
                     calibrateTipTemp(channel, tipSettings, CalibrationIndex_325) &&
                     calibrateTipTemp(channel, tipSettings, CalibrationIndex_400);

               if (success) {
                  TipSettings *ts = (TipSettings*)menuItems[selection].tipSettings;
                  ts->setThermisterCalibration(tipSettings);
                  menuItems[selection].modifiers |= MenuItem::Starred;
               }
            }
         }
         break;

         case ev_QuadRotate:
            selection += event.change;
            break;

         case ev_Ch1Release:
         case ev_Ch2Release:
            loopControl = complete;
            break;

         case ev_QuadHold:
         case ev_SelHold:
            event.type  = ev_None;
            loopControl = complete;
            break;

         default:
            refresh = false;
            break;
      }
   } while (loopControl == working);

   return event.type;
}

/**
 * Edit a non-volatile tip calibration setting.
 *
 * @param settings Data describing setting to change
 *
 * @return Exiting event
 */
bool Menus::editPidSetting(TipSettings &tipSettings) {

   // Max value truncated to nearest 100 (0.1 as float)
   static constexpr int MAX_VALUE = UINT16_MAX-UINT16_MAX%100;

   // Work with raw integer values  (internal format)
   BoundedInteger kp(    0, MAX_VALUE,    tipSettings.getRawKp());
   BoundedInteger ki(    0, MAX_VALUE,    tipSettings.getRawKi());
   BoundedInteger kd(    0, MAX_VALUE,    tipSettings.getRawKd());
   BoundedInteger iLimit(0, MAX_VALUE,    tipSettings.getRawILimit());

   int scratchKp     = kp;
   int scratchKi     = ki;
   int scratchKd     = kd;
   int scratchILimit = iLimit;

   enum {working, complete, fail} loopControl = working;
   Event event;
   BoundedInteger selection(0, 3, 0);

   bool modified = false;

   bool refresh  = true;
   do {
      if (refresh) {
         char stars[4] = {kp == scratchKp?' ':'*', ki == scratchKi?' ':'*', kd == scratchKd?' ':'*', iLimit == scratchILimit?' ':'*'};
         display.displayPidSettings(tipSettings.getTipName(), selection, stars, kp, ki, kd, iLimit);
         refresh = false;
      }

      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }

      // Assume refresh required
      refresh = true;

      switch (event.type) {

         case ev_SelRelease:
         case ev_QuadRelease:
            switch(selection) {
               case 0:
                  scratchKp = kp;
                  modified = true;
                  break;
               case 1:
                  scratchKi = ki;
                  modified = true;
                  break;
               case 2:
                  scratchKd = kd;
                  modified = true;
                  break;
               case 3:
                  scratchILimit = iLimit;
                  modified = true;
                  break;
            }
            break;

            case ev_QuadRotate:
               switch(selection) {
                  case 0:
                     kp += event.change*10;  // 0.010 steps
                     break;
                  case 1:
                     ki += event.change*1;  // 0.001 steps
                     break;
                  case 2:
                     kd += event.change*1;  // 0.001 steps
                     break;
                  case 3:
                     iLimit += event.change*100; // .100 steps
                     break;
               }
               break;

               case ev_QuadRotatePressed:
                  switch(selection) {
                     case 0:
                        kp += event.change*100;  // 0.100 steps
                        break;
                     case 1:
                        ki += event.change*10;  // 0.01 steps
                        break;
                     case 2:
                        kd += event.change*10;  // 0.01 steps
                        break;
                     case 3:
                        iLimit += event.change*1000; // 1.00 steps
                        break;
                  }
                  break;

         case ev_Ch1Release:
            selection--;
            break;

         case ev_Ch2Release:
            selection++;
            break;

         case ev_QuadHold:
         case ev_SelHold:
            event.type  = ev_None;
            loopControl = complete;
            break;

         default:
            refresh = false;
            break;
      }
   } while (loopControl == working);

   if (modified) {
      // Only update nonvolatile data as needed
      tipSettings.setRawPidControlValues(scratchKp, scratchKi, scratchKd, scratchILimit);
   }
   return modified;
}

/**
 * Edit a non-volatile tip PID setting.
 *
 * @return Exiting event
 */
EventType Menus::editPidSettings(const SettingsData &) {

   static constexpr unsigned modifiers = MenuItem::Starred;

   MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS] = {0};

   int tipsAllocated = tips.populateSelectedTips(menuItems, &TipSettings::isPidCalibrated);

   int  offset    = 0;
   BoundedInteger  selection{0, tipsAllocated-1, Tips::findTipInMenu(channels[1].getTip(), menuItems, tipsAllocated)};

   bool refresh  = true;
   enum {working, complete, fail} loopControl = working;
   Event event;

   do {
      if (refresh) {
         display.displayMenuList("PID Settings", menuItems, modifiers, offset, selection);
         refresh = false;
      }

      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }

      // Assume refresh required
      refresh = true;

      switch (event.type) {

         case ev_SelRelease:
         case ev_QuadRelease: {
            if (menuItems[selection].name == nullptr) {
               break;
            }
            TipSettings *ts = menuItems[selection].tipSettings;
            bool modified = editPidSetting(*ts);

            if (modified) {
               menuItems[selection].modifiers |= MenuItem::Starred;
            }
         }
         break;

         case ev_QuadRotate:
            selection += event.change;
            break;

         case ev_Ch1Release:
         case ev_Ch2Release:
            loopControl = complete;
            break;

         case ev_QuadHold:
         case ev_SelHold:
            event.type  = ev_None;
            loopControl = complete;
            break;

         default:
            refresh = false;
            break;
      }
   } while (loopControl == working);

   channels[1].refreshControllerParameters();
   channels[2].refreshControllerParameters();

   return event.type;
}

/**
 * Calculate a non-volatile tip PID settings.
 *
 * @return Exiting event
 */
EventType Menus::stepResponse(const SettingsData &) {

   static constexpr unsigned modifiers = MenuItem::Starred;

   MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS] = {0};

   int tipsAllocated = tips.populateSelectedTips(menuItems, &TipSettings::isPidCalibrated);

   int  offset    = 0;
   BoundedInteger  selection{0, tipsAllocated-1, Tips::findTipInMenu(channels[1].getTip(), menuItems, tipsAllocated)};

   bool refresh  = true;
   enum {working, complete, fail} loopControl = working;
   Event event;

   do {
      if (refresh) {
         display.displayMenuList("Step Response", menuItems, modifiers, offset, selection);
         refresh = false;
      }

      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }

      // Assume refresh required
      refresh = true;

      switch (event.type) {

         case ev_SelRelease:
         case ev_QuadRelease: {
            if (menuItems[selection].name == nullptr) {
               break;
            }
            Event ev = display.displayMessage(
                  "Step Response",
                  "This will drive\n"
                  "the tip at fixed\n"
                  "power for a period.\n\n"
                  "Press to start and end");
            if (ev.isSelRelease()) {
               TipSettings *settings = menuItems[selection].tipSettings;
               Channel &channel = channels[1];
               channel.setTip(settings);
               StepResponseDriver driver(channels[1]);
               driver.run(30);
            }
         }
         break;

         case ev_QuadRotate:
            selection += event.change;
            break;

         case ev_Ch1Release:
         case ev_Ch2Release:
            loopControl = complete;
            break;

         case ev_QuadHold:
         case ev_SelHold:
            event.type  = ev_None;
            loopControl = complete;
            break;

         default:
            refresh = false;
            break;
      }
   } while (loopControl == working);

   channels[1].refreshControllerParameters();
   channels[2].refreshControllerParameters();

   return event.type;
}

/**
 * Edit a available tips in non-volatile settings.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::selectAvailableTips(const SettingsData &) {

   int  offset    = 0;
   BoundedInteger  selection{0,TipSettings::NUMBER_OF_TIPS-1, 0};

   static constexpr unsigned modifiers = MenuItem::CheckBox|MenuItem::Starred;

   MenuItem tipMenuItems[TipSettings::NUMBER_OF_TIPS];

   bool         anyTipsSelected = false;
   TipSettings *defaultTip      = nullptr;

   Event event;

   while (!anyTipsSelected) {

      // Load available tips
      tips.populateTips(tipMenuItems);

      bool refresh = true;
      enum {working, complete} loopControl = working;

      do {
         if (refresh) {
            display.displayMenuList("  Enable tips", tipMenuItems, modifiers, offset, selection);
            refresh = false;
         }
         event = switchPolling.getEvent();
         if (event.type == ev_None) {
            continue;
         }

         // Assume refresh required
         refresh = true;

         switch (event.type) {

            case ev_SelRelease:
            case ev_QuadRelease: {
               MenuItem &mi = tipMenuItems[selection];
               mi.modifiers ^= MenuItem::CheckBoxSelected;
               if (((mi.modifiers&MenuItem::CheckBoxSelected) != 0) != (mi.object != nullptr)) {
                  mi.modifiers |= MenuItem::Starred;
               }
               else {
                  mi.modifiers &= ~MenuItem::Starred;
               }
               break;
            }

            case ev_QuadRotate:
               selection += event.change;
               break;

            case ev_QuadHold:
            case ev_SelHold:
               event.type = ev_None;
               loopControl = complete;
               break;

            case ev_Ch1Release:
            case ev_Ch2Release:
               loopControl = complete;
               break;

            default:
               refresh = false;
               break;
         }
      } while (loopControl == working);

      // Create or delete non-volatile tip settings as needed
      anyTipsSelected = false;
      for(unsigned index=0; index<TipSettings::NUMBER_OF_TIPS; index++) {
         TipSettings *tip = nullptr;
         if ((tipMenuItems[index].modifiers & MenuItem::CheckBoxSelected) != 0) {
            // Make sure setting has been allocated for this tip
            tip = tips.findOrAllocateTipSettings(tipMenuItems[index].name);
         }
         else {
            // Delete existing setting if necessary
            tip = tips.findTipSettings(tipMenuItems[index].name);
            if (tip != nullptr) {
               StringFormatter_T<40> prompt;
               prompt.write("Delete calibration\ndata for ").write(tip->getTipName()).write(" ?");

               if (!(tip->isTemperatureCalibrated() || tip->isPidCalibrated()) || confirmAction(prompt.toString())) {
                  // Deleting this tip
                  tip->freeEntry();
                  tip = nullptr;
               }
            }
         }
         if ((tip != nullptr) && !anyTipsSelected) {
            defaultTip = tip;
            anyTipsSelected = true;
         }
      }
      if (!anyTipsSelected) {
         display.displayMessage("Error", "At least 1 tip must\nbe selected");
      }
   }
   channels[1].checkTipSelected(defaultTip);
   channels[2].checkTipSelected(defaultTip);

   return event.type;
}

/**
 * Confirm action from user
 *
 * @param prompt Prompt to display
 *
 * @return True if confirmed
 * @return false if cancelled
 */
bool Menus::confirmAction(const char *prompt) {

   static const char *options[] = {
         "Yes", "No", nullptr,
   };

   bool doUpdate = true;
   BoundedInteger selection{0, 1, 1};
   do {
      if (doUpdate) {
         display.displayChoice("Warning", prompt, options, selection);
      }
      Event event = switchPolling.getEvent();
      switch(event.type) {
         case ev_QuadRotate:
            selection += event.change;
            doUpdate = true;
            break;

         case ev_SelRelease:
         case ev_QuadRelease:
            return (selection == 0);

         default:
            break;
      }
   } while(true);
}

/**
 */
EventType Menus::displayChannelStatuses(const SettingsData &) {

   BoundedInteger selection{0,1,1};
   do {
      display.displayChannelStatuses();
      Event event = switchPolling.getEvent();
      switch(event.type) {
         case ev_SelRelease:
         case ev_QuadRelease:
            return ev_None;

         default:
            break;
      }
   } while(true);
}

EventType Menus::runHeater(const SettingsData &) {

   Channel &ch = channels[1];

   BoundedInteger  dutyCycle{0, 100, 0};

   bool refresh  = true;
   enum {working, complete, fail} loopControl = working;
   Event event;

   ch.setDutyCycle(dutyCycle);
   ch.setState(ch_fixedPower);

   do {
      if (refresh || control.needsRefresh()) {
         display.displayHeater(ch, dutyCycle);
         refresh = false;
      }

      event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }

      // Assume refresh required
      refresh = true;

      switch (event.type) {

         case ev_QuadRotate:
            dutyCycle += event.change;
            ch.setDutyCycle(dutyCycle);
            ch.restartIdleTimer();
            break;

         case ev_None:
            refresh = false;
            break;

         default:
            // Exit on anything else
            event.type  = ev_None;
            loopControl = complete;
            break;
      }
   } while (loopControl == working);

   ch.setDutyCycle(0);
   ch.setState(ch_off);

   return ev_None;
}

/**
 * Display and execute top-level menu
 */
void Menus::settingsMenu() {

   static const SettingsData settingsData[] = {
         {"Heater",                   runHeater,                                                        },
         {"Channel 1\nSetback temp.", editTemperature,      nvinit.ch1Settings.setbackTemperature, 1    }, // C
         {"Channel 2\nSetback temp.", editTemperature,      nvinit.ch2Settings.setbackTemperature, 1    }, // C
         {"Channel 1\nIdle time",     editTime,             nvinit.ch1Settings.setbackTime,        10   }, // seconds
         {"Channel 2\nIdle time",     editTime,             nvinit.ch2Settings.setbackTime,        10   }, // seconds
         {"Channel 1\nSafety time",   editTime,             nvinit.ch1Settings.safetyOffTime,      10   }, // seconds
         {"Channel 2\nSafety time",   editTime,             nvinit.ch2Settings.safetyOffTime,      10   }, // seconds
         {"Tip Selection",            selectAvailableTips                                               },
         {"Temp Calibration",         calibrateTipTemps                                                 },
         {"Step Response",            stepResponse                                                      },
         {"PID Manual",               editPidSettings                                                   },
         {"Channel Status",           displayChannelStatuses                                            },
   };

   //   console.write("Size    = ").writeln(sizeof(settingsData));
   //   console.write("Address = ").writeln(&settingsData);

   static const MenuItem items[] = {
         {"Heater", },
         {"Ch1 Setback temp.", },
         {"Ch2 Setback temp.", },
         {"Ch1 Idle time",     },
         {"Ch2 Idle time",     },
         {"Ch1 Safety time",   },
         {"Ch1 Safety time",   },
         {"Tip Selection",     },
         {"Temp Calibration",  },
         {"Step Response",     },
         {"Pid Manual set",    },
         {"Channel Status",    },
   };

   int  offset    = 0;
   BoundedInteger  selection{0,(int)(sizeof(settingsData)/sizeof(settingsData[0])-1), 0};

   bool refresh = true;
   for (;;) {
      if (refresh) {
         display.displayMenuList("  Settings", items, offset, selection);
         refresh = false;
      }
      Event event = switchPolling.getEvent();
      if (event.type == ev_None) {
         continue;
      }

      // Assume refresh required
      refresh = true;

      //      console.write(getEventName(event)).write(" : ").writeln(event.change);

      EventType key = ev_None;

      switch (event.type) {

         case ev_SelRelease:
         case ev_QuadRelease:
            do {
               key = settingsData[selection].handler(settingsData[selection]);
               switch(key) {
                  case ev_Ch1Release:
                     selection--;
                     break;
                  case ev_Ch2Release:
                     selection++;
                     break;
                  default:
                     break;
               }
            } while((key == ev_Ch1Release)||(key == ev_Ch2Release));
            break;

         case ev_Ch1Release:
            selection--;
            break;

         case ev_Ch2Release:
            selection++;
            break;

         case ev_QuadRotate:
            selection += event.change;
            break;

         case ev_QuadHold:
         case ev_SelHold:
            return;

         default:
            refresh = false;
            break;
      }
   }
}

