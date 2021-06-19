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

using namespace USBDM;

/**
 * Edit a non-volatile time setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::editTime(const SettingsData &data) {
   static constexpr int MAX_TIME = (99*60+50)*1000; // in milliseconds

   Event event;;

   BoundedInteger<0, MAX_TIME> scratch(*data.settingInt);

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
            *data.settingInt = scratch;
            unchanged = scratch;
            doRefresh = true;
            break;

         case ev_QuadRotate:
            scratch += event.change * data.increment;
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

   Event event;;

   BoundedInteger<0, INT_MAX> scratch = round(*data.settingFloat * 1000);

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
 * Edit a non-volatile temperatire setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::editTemp(const SettingsData &data) {

   Event event;;

   BoundedInteger<Control::MIN_TEMP, Control::MAX_TEMP> scratch(*data.settingInt);

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
            *data.settingInt = scratch;
            unchanged = scratch;
            doRefresh = true;
            break;

         case ev_QuadRotate:
            // Increment by multiple of scale with forced rounding
            scratch += event.change * data.increment;
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
 * Edit a non-volatile tip setting.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::editTipSettings(const SettingsData &) {

   Channel &channel = channels[1];

   static constexpr unsigned modifiers = Display::MenuItem::Starred;

   Display::MenuItem menuItems[TipSettings::NUM_TIP_SETTINGS] = {0};

   int tipsAllocated = 0;

   // Load tip information for menu from nv-storage
   for(unsigned index=0; index<TipSettings::NUM_TIP_SETTINGS; index++) {
      const TipSettings       &ts = tips.getTip(index);
      Display::MenuItem &mi = menuItems[tipsAllocated];
      if (!ts.isFree()) {
         mi.name   = ts.getTipName();
         mi.cobject = &ts;
         if (ts.isCalibrated()) {
            // Mark calibrated tips with star
            mi.modifiers |= Display::MenuItem::Starred;
         }
         tipsAllocated++;
      }
   }

   // Sort the menu
   Display::MenuItem::sort(menuItems, tipsAllocated);

   int  offset    = 0;
   BoundedInteger<0,TipSettings::NUM_TIP_SETTINGS-1>  selection{0};

   // This is a dummy settings object that is NOT in nv-storage
   // Made static to avoid warnings about non-initialised object
   static TipSettings settings;

   bool refresh  = true;
   enum {working, complete, fail} loopControl = working;
   Event event;

   do {
      if (refresh) {
         display.displayMenuList("Tip Calibration", menuItems, modifiers, offset, selection);
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
                  "to Target value\n"
                  "Press'n'hold to abort");

            if (ev.isSelRelease()) {
               settings.tipNameIndex = tips.getTip(selection).tipNameIndex;
               bool success = calibrate(channel, settings, TipSettings::Calib_250) &&
                     calibrate(channel, settings, TipSettings::Calib_350) &&
                     calibrate(channel, settings, TipSettings::Calib_400);

               if (success) {
                  TipSettings *ts = (TipSettings*)menuItems[selection].object;
                  ts->setPoints(settings);
                  menuItems[selection].modifiers |= Display::MenuItem::Starred;
               }
            }
         }
         break;

         case ev_QuadRotate:
            selection += event.change;
            if (selection >= tipsAllocated) {
               selection = tipsAllocated-1;
            }
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
 * Edit a available tips in non-volatile settings.
 *
 * @param data Data describing setting to change
 *
 * @return Exiting event
 */
EventType Menus::selectAvailableTips(const SettingsData &) {

   int  offset    = 0;
   BoundedInteger<0,TipSettings::NUMBER_OF_TIPS-1>  selection{0};

   static constexpr unsigned modifiers = Display::MenuItem::CheckBox|Display::MenuItem::Starred;

   Display::MenuItem tipMenuItems[TipSettings::NUMBER_OF_TIPS];

   // Copy tip information to menu settings
   for(TipSettings::TipNameIndex index=0; index<TipSettings::NUMBER_OF_TIPS; index++) {
      tipMenuItems[index].name      = TipSettings::tipNames[index];
      tipMenuItems[index].modifiers = 0;
      tipMenuItems[index].object    = nullptr;

      TipSettings *tip = tips.findTipSettings(index);
      if (tip != nullptr) {
         // Non-volatile tip settings have already been allocated
         tipMenuItems[index].modifiers |= Display::MenuItem::CheckBoxSelected;
         tipMenuItems[index].object    = tip;
      }
   }

   // Sort the menu
   Display::MenuItem::sort(tipMenuItems, TipSettings::NUMBER_OF_TIPS);

   bool refresh = true;
   enum {working, complete} loopControl = working;
   Event event;

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
            Display::MenuItem &mi = tipMenuItems[selection];
            mi.modifiers ^= Display::MenuItem::CheckBoxSelected;
            if (((mi.modifiers&Display::MenuItem::CheckBoxSelected) != 0) != (mi.object != nullptr)) {
               mi.modifiers |= Display::MenuItem::Starred;
             }
            else {
               mi.modifiers &= ~Display::MenuItem::Starred;
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
   for(unsigned index=0; index<TipSettings::NUMBER_OF_TIPS; index++) {

      if ((tipMenuItems[index].modifiers & Display::MenuItem::CheckBoxSelected) != 0) {
         // Make sure setting has been allocated
         tips.findOrAllocateTipSettings(tipMenuItems[index].name);
      }
      else {
         // Delete existing setting if necessary
         TipSettings *tip = tips.findTipSettings(tipMenuItems[index].name);
         if (tip != nullptr) {
            StringFormatter_T<40> prompt;
            prompt.write("Delete calibration\ndata for ").write(tip->getTipName()).write(" ?\n");

            if (!tip->isCalibrated() || confirmAction(prompt.toString())) {
               tip->freeEntry();
            }
         }
      }
   }
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
   BoundedInteger<0,1> selection{1};
   do {
      if (doUpdate) {
         display.displayChoice("Warning\n", prompt, options, selection);
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
 * Display and execute top-level menu
 */
void Menus::settingsMenu() {

   static const SettingsData settingsData[] = {
         {"Tip Selection",            selectAvailableTips                                                },
         {"Tip Calibration",          editTipSettings                                                    },
         {"Channel 1\nSetback temp.", editTemp,          nvinit.ch1Settings.setbackTemperature, 1        },
         {"Channel 2\nSetback temp.", editTemp,          nvinit.ch2Settings.setbackTemperature, 1        },
         {"Channel 1\nIdle time",     editTime,          nvinit.ch1Settings.setbackTime,        10000    },
         {"Channel 2\nIdle time",     editTime,          nvinit.ch2Settings.setbackTime,        10000    },
         {"Channel 1\nSafety time",   editTime,          nvinit.ch1Settings.safetyOffTime,      10000    },
         {"Channel 2\nSafety time",   editTime,          nvinit.ch2Settings.safetyOffTime,      10000    },
         {"PID Kp",                   editFloat,         nvinit.pidSettings.kp,                 50       },
         {"PID Ki",                   editFloat,         nvinit.pidSettings.ki,                 50       },
         {"PID Kd",                   editFloat,         nvinit.pidSettings.kd,                 5        },
         {"PID I Limit",              editFloat,         nvinit.pidSettings.iLimit,             500      },
   };

   //   console.write("Size    = ").writeln(sizeof(settingsData));
   //   console.write("Address = ").writeln(&settingsData);

   const Display::MenuItem items[] = {
         {"Tip Selection",     },
         {"Tip Calibration",   },
         {"Ch1 Setback temp.", },
         {"Ch2 Setback temp.", },
         {"Ch1 Idle time",     },
         {"Ch2 Idle time",     },
         {"Ch1 Safety time",   },
         {"Ch1 Safety time",   },
         {"PID Kp",            },
         {"PID Ki",            },
         {"PID Kd",            },
         {"PID I Limit",       },
   };

   int  offset    = 0;
   BoundedInteger<0,sizeof(settingsData)/sizeof(settingsData[0])-1>  selection{0};

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

bool Menus::calibrate(Channel &ch, TipSettings &tipsettings, TipSettings::Calib stage) {
   static const unsigned targetTemperatures[] = {250, 350, 400};

   if (tipsettings.isFree()) {
      return false;
   }
   //   console.setFloatFormat(0, Padding_LeadingSpaces, 2);
   //   console.write("Test = ").writeln(1.2f);

   bool doUpdate = true;

   BoundedInteger<Control::MIN_TEMP, Control::MAX_TEMP> controlledTemperature = targetTemperatures[stage];

   ch.setState(ch_active);
   ch.setUserTemperature(controlledTemperature);

   StringFormatter_T<16> title;
   title.write("Stage ").write(stage+1).write(" - ").write(tipsettings.getTipName());

   enum {working, complete, fail} loopControl = working;
   do {
      if (doUpdate || control.needsRefresh()) {

         display.displayCalibration(title.toString(), ch, targetTemperatures[stage]);
      }
      Event event = switchPolling.getEvent();
      switch(event.type) {
         case ev_QuadRotate:
            controlledTemperature += event.change;
            ch.setUserTemperature(controlledTemperature);
            doUpdate = true;
            break;

         case ev_SelRelease:
         case ev_QuadRelease:
            // Save values
            tipsettings.coldJunctionTemperature.set(stage,ch.coldJunctionTemperature.getTemperature());
            tipsettings.thermocoupleVoltage.set(stage, ch.tipTemperature.getVoltage());
            loopControl = complete;
            break;

         case ev_SelHold:
         case ev_QuadHold:
            loopControl = fail;
            break;

         default:
            break;
      }
   } while(loopControl == working);

   ch.setState(ch_off);
   return loopControl == complete;
}

