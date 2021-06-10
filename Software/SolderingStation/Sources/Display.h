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
   Display() {}

   /**
    * Initialise the display
    */
   void initialise() {
//      oled.clearDisplay();
//      oled.refreshImage();
   }

   /**
    * Update OLED display
    *
    * @param ch1              Channel 1 properties
    * @param ch2              Channel 2 properties
    * @param selectedChannel  The currently selected channel
    */
   void displayTools();

   void displayTimeMenuItem(const char *name, unsigned seconds);

   void displayTemperatureMenuItem(const char *name, unsigned temperature);
};

extern Display display;

#endif /* SOURCES_DISPLAY_H_ */
