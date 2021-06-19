/*
 * Display.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "Display.h"
#include "Channels.h"

using namespace USBDM;

/**
 * Display channel information
 *
 * @param ch1              Channel 1 properties
 * @param ch2              Channel 2 properties
 * @param selectedChannel  The currently selected channel
 */
void Display::displayTools() {
   using namespace USBDM;

   static constexpr unsigned LEFT_OFFSET  = 1;
   static constexpr unsigned RIGHT_OFFSET = 1+(oled.WIDTH+1)/2;

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   oled.clearDisplay();

   oled.setPadding(Padding_LeadingSpaces).setWidth(3);

   oled.setFont(fontVeryLarge);
   if ((ch1.getState() == ch_noTip) || (ch1.getState() == ch_overload)) {
      oled.moveXY(LEFT_OFFSET+4, 10).write("---");
   }
   else {
      int currentTemp = ch1.getCurrentTemperature();
      if (currentTemp>999) {
         currentTemp = 999;
      }
      oled.moveXY(LEFT_OFFSET, 10).write(currentTemp);
      oled.setFont(fontMedium);
      oled.moveXY(LEFT_OFFSET+47, 15).write("C");
   }

   oled.setFont(fontVeryLarge);
   if ((ch2.getState() == ch_noTip) || (ch2.getState() == ch_overload)) {
      oled.moveXY(RIGHT_OFFSET+4, 10).write("---");
   }
   else {
      int currentTemp = ch2.getCurrentTemperature();
      if (currentTemp>999) {
         currentTemp = 999;
      }
      oled.moveXY(RIGHT_OFFSET, 10).write(currentTemp);
      oled.setFont(fontMedium);
      oled.moveXY(RIGHT_OFFSET+47, 15).write("C");
   }

   switch (channels.getSelectedChannelNumber()) {
      case 1:
         oled.drawRect(LEFT_OFFSET-1,  12, LEFT_OFFSET+56,  12+23, WriteMode_Xor);
         break;
      case 2:
         oled.drawRect(RIGHT_OFFSET-1,  12, RIGHT_OFFSET+56,  12+23, WriteMode_Xor);
         break;
   }

   oled.setFont(fontMedium);
   oled.moveXY(LEFT_OFFSET, 0).write(ch1.getStateName());
   oled.moveXY(RIGHT_OFFSET, 0).write(ch2.getStateName());

   oled.setFont(fontLarge);
   oled.moveXY(LEFT_OFFSET,  40).write("P").setWidth(1).write(ch1.getPreset()).
         write(ch1.isTempModified()?"*:":" :").setWidth(3).write(ch1.getUserTemperature());
   oled.moveXY(RIGHT_OFFSET, 40).write("P").setWidth(1).write(ch2.getPreset()).
         write(ch2.isTempModified()?"*:":" :").setWidth(3).write(ch2.getUserTemperature());

   float ch1Power = ch1.getPower();
   float ch2Power = ch2.getPower();

   oled.setFont(fontSmall);

   oled.moveXY(LEFT_OFFSET+1,  56).write(ch1.getTipName());
   oled.moveXY(RIGHT_OFFSET+1, 56).write(ch2.getTipName());

   oled.setFloatFormat(1, Padding_LeadingSpaces, 4);
   oled.moveXY(LEFT_OFFSET+18,  56).write(ch1Power).write('W');
   oled.moveXY(RIGHT_OFFSET+18, 56).write(ch2Power).write('W');
   if (ch1Power>0) {
      oled.drawRect(LEFT_OFFSET,   55, LEFT_OFFSET-1+ch1Power/2,  63, WriteMode_Xor);
   }
   if (ch2Power>0) {
      oled.drawRect(RIGHT_OFFSET,  55, RIGHT_OFFSET-1+ch2Power/2, 63, WriteMode_Xor);
   }
   oled.drawVerticalLine(RIGHT_OFFSET-4, 0, oled.HEIGHT-1, WriteMode_Write);

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display time menu item
 *
 * @param description      Description to display at top of screen.  May include a single newline to split into 2 lines.
 * @param seconds          Time in seconds to display
 * @param modified         Indicates item has been modified since saving - adds indicator to display
 */
void Display::displayTimeMenuItem(const char *description, unsigned ms, bool modified) {
   oled.clearDisplay();

   oled.setFont(fontLarge);
   oled.moveXY(0, 0).writeln(description);

   oled.setFont(fontVeryLarge);
   oled.moveXY(0, 30).write(modified?'*':' ');

   int seconds = ms/1000;

   if (ms>0) {
      oled.setPadding(Padding_LeadingSpaces).setWidth(2).write(seconds/60).write("m");
      oled.setPadding(Padding_LeadingZeroes).setWidth(2).write(seconds%60).write("s");
   }
   else {
      oled.write(" Off");
   }

//   oled.moveXY(28, 55).setFont(fontSmall).write(" <-           ->");

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display temperature menu item
 *
 * @param description   Description to display at top of screen.  May include a single newline to split into 2 lines.
 * @param temperature   Temperature to display
 * @param modified      Indicates item has been modified since saving - adds indicator to display
 */
void Display::displayFloatMenuItem(const char *description, int value, bool modified) {
   oled.clearDisplay();

   oled.setFont(fontLarge);
   oled.moveXY(0, 0).writeln(description);

   oled.setFont(fontVeryLarge);
   oled.moveXY(10, 30).write(modified?'*':' ');
   oled.write(value/1000).write('.');
   oled.setPadding(Padding_LeadingZeroes).setWidth(3).write(value%1000);

//   oled.moveXY(28, 55).setFont(fontSmall).write(" <-           ->");

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display temperature menu item
 *
 * @param description   Description to display at top of screen.  May include a single newline to split into 2 lines.
 * @param temperature   Temperature to display
 * @param modified      Indicates item has been modified since saving - adds indicator to display
 */
void Display::displayTemperatureMenuItem(const char *description, unsigned temperature, bool modified) {
   oled.clearDisplay();

   oled.setFont(fontLarge);
   oled.moveXY(0, 0).writeln(description);

   oled.setFont(fontVeryLarge);
   oled.setPadding(Padding_LeadingSpaces).setWidth(4);
   oled.moveXY(0, 30).write(modified?'*':' ').write(temperature);
   oled.moveXY(80, 35).setFont(fontMedium).write("C");

//   oled.moveXY(28, 55).setFont(fontSmall).write(" <-           ->");

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display a menu list with selected item
 *
 * @param[in]     title         Title to display at top of screen
 * @param[in]     items         Array of menu items.  Must have at least MIN_MENU_ENTRIES items.
 * @param[in]     modifiersUsed Modifiers that may be applied to items (for sizing)
 * @param[in/out] offset        Offset into list for display i.e. first item on visible menu.
 * @param[in]     selection     Selected item index
 */
void Display::displayMenuList(const char *title, MenuItem const items[], unsigned modifiersUsed, int &offset, int selection) {
   Font &font = fontSmall;

   oled.clearDisplay();

   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());

   oled.moveXY(0, oled.getY()+1);
   oled.setFont(font);

   if (selection < offset) {
      offset = selection;
   }
   else if ((selection-offset) >= MIN_MENU_ENTRIES) {
      offset = selection-(MIN_MENU_ENTRIES-1);
   }

   for(int line=0; line<MIN_MENU_ENTRIES; line++) {
      char prefix[10] = {0};
      char *p = prefix;
      if (modifiersUsed & MenuItem::CheckBox) {
         *p++ = '[';
         *p++ = ' ';
         *p++ = ']';
      }
      if (modifiersUsed & MenuItem::Starred) {
         *p = ' ';
      }
      auto item = items[line+offset];
      if (item.name == nullptr) {
         break;
         item = "";
      }
      if (item.modifiers&MenuItem::CheckBoxSelected) {
         prefix[1] = 'X';
      }
      if (item.modifiers&MenuItem::Starred) {
         *p = '*';
      }
      int menuY = oled.getY();
      oled.write(prefix);
      oled.writeln(item.name);
      if ((line+offset) == selection) {
         oled.drawRect(0,  menuY, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
      }
   }

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display message with a few selection options.
 *
 * @param[in] title     Title to display at top of screen
 * @param[in] prompt    Prompt for selection
 * @param[in] options   Options to display
 * @param[in] selection Selected item
 */
void Display::displayChoice(const char *title, const char *prompt, const char *options[], int selection) {
   Font &font = fontSmall;

   oled.clearDisplay();

   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());

   oled.setFont(font);
   oled.writeln(prompt);

   for(int line=0; line<MIN_MENU_ENTRIES; line++) {
      if (options[line] == nullptr) {
         break;
      }
      int menuY = oled.getY();
      oled.writeln(options[line]);
      if (line == selection) {
         oled.drawRect(0,  menuY, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
      }
   }

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display message and waits for user action
 *
 * @param[in] title     Title to display at top of screen
 * @param message       Message to display
 *
 * @return Event type that exited wait
 */
Event Display::displayMessage(const char *title, const char *message) {
   Font &font = fontSmall;

   oled.clearDisplay();

   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());

   oled.moveXY(0, oled.getY()+2);
   oled.setFont(font);
   oled.writeln(message);

   oled.refreshImage();

   oled.resetFormat();

   Event event;
   while (!event.isSelHold() && !event.isSelRelease()) {
      event = switchPolling.getEvent();
   }
   return event;
}

void Display::displayCalibration(const char *title, Channel &ch, unsigned targetTemperature) {

   oled.clearDisplay();
   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());

   oled.moveXY(0, oled.getY()+4);
   oled.setFont(fontSmall);
   oled.write("Target       ").write(targetTemperature).writeln(" C");

   oled.moveXY(0, oled.getY()+2);
   oled.write("Measured     ").write(ch.getCurrentTemperature()).writeln(" C");

   oled.moveXY(0, oled.getY()+2);
   oled.write("Controlled   ").write(ch.getUserTemperature()).writeln(" C");

   oled.moveXY(0, oled.getY()+5);
//   oled.write(" Power ").write(ch.getPower());
   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.write("Vt ").write(1000*ch.tipTemperature.getVoltage()).write("mV");

   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.write(", Tc ").write(ch.coldJunctionTemperature.getTemperature()).write("C");

   oled.refreshImage();

   oled.resetFormat();
}

