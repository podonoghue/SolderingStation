/*
 * DIsplay.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_DISPLAY_H_
#define SOURCES_DISPLAY_H_

#include "Peripherals.h"
#include "i2c.h"
#include "Oled.h"
#include "Channel.h"

class Display {

private:
   USBDM::I2c0  i2c{};
   USBDM::Oled  oled{i2c};
   unsigned     activeChannel = 0;

public:
   static constexpr int MIN_MENU_ENTRIES = 8;

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

   /**
    * Display a menu list with selected item
    *
    * @param items      Array of string describing each item.  Must have at least MIN_MENU_ENTRIES items.
    * @param offset     Offset into list for display i.e. first item on menu
    * @param selection  Current selected it.
    */
   void displayMenuList(const char *items[], int offset, int selection);
};

extern Display display;

#endif /* SOURCES_DISPLAY_H_ */
