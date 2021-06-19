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
#include "Channel.h"
#include "SwitchPolling.h"

class Display {

private:
   USBDM::I2c0  i2c{};
   USBDM::Oled  oled{i2c};
   unsigned     activeChannel = 0;

public:
   static constexpr int MIN_MENU_ENTRIES = 7;

   Display() {}

   /**
    * Initialise the display
    */
   void initialise() {
//      oled.clearDisplay();
//      oled.refreshImage();
   }

   /**
    * Display channel information
    *
    * @param ch1              Channel 1 properties
    * @param ch2              Channel 2 properties
    * @param selectedChannel  The currently selected channel
    */
   void displayTools();

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

   class MenuItem {
   public:
      static constexpr unsigned CheckBox         = 1<<0;
      static constexpr unsigned CheckBoxSelected = 1<<1;
      static constexpr unsigned Starred          = 1<<2;

      const char *name;
      unsigned modifiers;
      union {
         void *object;
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
    * Display message
    *
    * @param[in] title     Title to display at top of screen
    * @param message       Message to display
    */
   Event displayMessage(const char *title, const char *message);

   void displayCalibration(const char *title, Channel &ch, unsigned targetTemperature);

};

extern Display display;

#endif /* SOURCES_DISPLAY_H_ */
