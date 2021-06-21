/*
 * DIsplay.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_DISPLAY_H_
#define SOURCES_DISPLAY_H_

#include "string.h"
#include "Peripherals.h"
#include "i2c.h"
#include "Oled.h"
#include "SwitchPolling.h"
#include "TipSettings.h"

class Channel;

class MenuItem {
public:
   static constexpr unsigned CheckBox         = 1<<0;
   static constexpr unsigned CheckBoxSelected = 1<<1;
   static constexpr unsigned Starred          = 1<<2;

   const char *name;
   unsigned modifiers;
   union {
      void *object;
      TipSettings *tipSettings;
      const void *cobject;
      int  intValue;
   };
   MenuItem() : name(nullptr), modifiers(0), intValue(0) {}
   MenuItem(const char *name) : name(name), modifiers(0), intValue(0) {}
   MenuItem(unsigned modifiers, const char *name, int value=0) : name(name), modifiers(modifiers), intValue(value) {}

   static int compare(const void *left, const void *right) {
      const MenuItem *l = (const MenuItem *)left;
      const MenuItem *r = (const MenuItem *)right;
      return strcmp(l->name, r->name);
   }

   static void sort(MenuItem items[], int size) {
      qsort(items, size, sizeof(MenuItem), compare);
   }
};


class Display {

private:
   USBDM::I2c0  i2c{};
   USBDM::Oled  oled{i2c};
   unsigned     activeChannel = 0;

   /**
    * Display information about one channel on OLED.
    * Used to draw left and right halves of screen
    *
    * @param ch         Channel to display
    * @param selected   Whether the channel is currently selected
    * @param offset     Y offset for display
    */
   void displayChannel(Channel &ch, bool selected, unsigned offset);

public:

   /// Minimum number of Menu items for display (arrays must be at least this size)
   static constexpr int MIN_MENU_ENTRIES = 6;

   /**
    * Constructor
    */
   Display() {}

   /**
    * Initialise the display
    */
   void initialise() {}

   /**
    * Display channel information on OLED
    */
   void displayChannels();

   /**
    * Display floating point menu item
    *
    * @param description   Description to display at top of screen.  May include a single newline to split into 2 lines.
    * @param value         Value to display.  This value is /1000 for scaling as a float value
    * @param modified      Indicates item has been modified since saving - adds indicator to display
    */
   void displayFloatMenuItem(const char *description, int value, bool modified);

   /**
    * Display time menu item
    *
    * @param description      Description to display at top of screen.  May include a single newline to split into 2 lines.
    * @param seconds          Time in seconds to display
    * @param modified         Indicates item has been modified since saving - adds indicator to display
    */
   void displayTimeMenuItem(const char *description, unsigned seconds, bool modified);

   /**
    * Display temperature menu item
    *
    * @param description   Description to display at top of screen.  May include a single newline to split into 2 lines.
    * @param temperature   Temperature to display
    * @param modified      Indicates item has been modified since saving - adds indicator to display
    */
   void displayTemperatureMenuItem(const char *description, unsigned temperature, bool modified);

   /**
    * Display a menu list with selected item
    *
    * @param[in]     title         Title to display at top of screen
    * @param[in]     items         Array of menu items.  Must have at least MIN_MENU_ENTRIES items.
    * @param[in]     modifiersUsed Modifiers that may be applied to items (for sizing)
    * @param[in/out] offset        Offset into list for display i.e. first item on visible menu.
    * @param[in]     selection     Selected item index
    */
   void displayMenuList(const char *title, MenuItem const items[], unsigned modifiersUsed, int &offset, int selection);

   /**
    * Display a menu list with selected item
    *
    * @param[in]     items         Array of menu items.  Must have at least MIN_MENU_ENTRIES items.
    * @param[in]     modifiersUsed Modifiers that may be applied to items (for sizing)
    * @param[in/out] offset        Offset into list for display i.e. first item on menu
    * @param[in]     selection     Selected item index
    */
   void displayMenuList(MenuItem const items[], unsigned modifiersUsed, int &offset, int selection) {
      displayMenuList(nullptr, items, modifiersUsed, offset, selection);
   }

   /**
    * Display a menu list with selected item
    *
    * @param[in]     items         Array of menu items.  Must have at least MIN_MENU_ENTRIES items.
    * @param[in/out] offset        Offset into list for display i.e. first item on visible menu.
    * @param[in]     selection     Selected item index
    */
   void displayMenuList(MenuItem const items[], int &offset, int selection) {
      displayMenuList(nullptr, items, 0, offset, selection);
   }

   /**
    * Display a menu list with selected item
    *
    * @param[in]     title         Title to display at top of screen
    * @param[in]     items         Array of menu items.  Must have at least MIN_MENU_ENTRIES items.
    * @param[in/out] offset        Offset into list for display i.e. first item on visible menu.
    * @param[in]     selection     Selected item index
    */
   void displayMenuList(const char *title, MenuItem const items[], int &offset, int selection) {
      displayMenuList(title, items, 0, offset, selection);
   }

   /**
    * Display message with a few selection options.
    *
    * @param[in] title     Title to display at top of screen
    * @param[in] prompt    Prompt for selection
    * @param[in] options   Options to display
    * @param[in] selection Selected item
    */
   void displayChoice(const char *title, const char *prompt, const char *options[], int selection);

   /**
    * Display message and waits for user action
    *
    * @param[in] title     Title to display at top of screen
    * @param[in] message   Message to display
    *
    * @return Event type that exited wait
    */
   Event displayMessage(const char *title, const char *message);

   /**
    * Display PID setting values
    *
    * @param tipname    Name of tip to display
    * @param selection  Index of selected item
    * @param stars      Array containing character to prefic menu item with
    * @param kp         PID Kp value
    * @param ki         PID Ki value
    * @param kd         PID Kd value
    * @param iLimit     PID I limit value
    *
    * @return
    */
   EventType displayPidSettings(const char *tipname, unsigned selection, char stars[4], int kp, int ki, int kd, int iLimit);

   /**
    * Display calibration information for tip during calibration sequence
    *
    * @param title               Title to display
    * @param ch                  Channel being used for calibration
    * @param targetTemperature   Target temperature for this calibration stage
    */
   void displayCalibration(const char *title, Channel &ch, unsigned targetTemperature);
};

extern Display display;

#endif /* SOURCES_DISPLAY_H_ */
