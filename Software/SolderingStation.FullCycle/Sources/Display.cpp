/*
 * Display.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "Display.h"
#include "Channels.h"
#include "Control.h"

using namespace USBDM;

/**
 * Display information about one channel on OLED.
 * Used to draw left and right halves of screen
 *
 * @param ch         Channel to display
 * @param offset     Y offset for display
 */
void Display::displayChannelStatus(Channel &ch, unsigned offset) {
   using namespace USBDM;

   oled.moveXY(0, offset);
   oled.setFont(fontMedium);

   float Tc = ch.getThermisterTemperature();
   float Tt = ch.getThermocoupleTemperature();

   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.write("Rc ").write(ch.getThermisterResistance()/1000).write("k,");
   oled.write(Tc).writeln("C");

   oled.setFloatFormat(2, Padding_LeadingSpaces, 0);
   oled.write("Vt ").write(round(ch.getThermocoupleVoltage()*100000)/100).write("mV,");
   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.write(Tt).writeln("C");
   oled.write("T = ").write(Tc+Tt).writeln("C");
}

/**
 * Display information about channels on OLED.
 */
void Display::displayChannelStatuses() {
   static constexpr int middle = (Oled::HEIGHT/2)-5;

   oled.clearDisplay();

   displayChannelStatus(channels[1], 0);
   displayChannelStatus(channels[2], middle+1);

   oled.drawHorizontalLine(0, Oled::WIDTH, middle-1, WriteMode_Write);

   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.write("Chip ").write(control.getChipTemperature()).write("C");

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display information about one channel on OLED.
 * Used to draw left and right halves of screen
 *
 * @param ch         Channel to display
 * @param selected   Whether the channel is currently selected
 * @param offset     X offset for display
 */
void Display::displayChannel(Channel &ch, bool selected, unsigned offset) {
   using namespace USBDM;

   oled.setFont(fontMedium);
   oled.moveXY(offset, 0).write(ch.getStateName());

   oled.setPadding(Padding_LeadingSpaces).setWidth(3);

   oled.setFont(fontVeryLarge);
   if ((ch.getState() == ch_noTip) || (ch.getState() == ch_overload)) {
      oled.moveXY(offset+6, 8).write("---");
   }
   else {
      int currentTemp = round(ch.getCurrentTemperature());
      if (currentTemp>999) {
         currentTemp = 999;
      }
      oled.moveXY(offset+2, 8).write(currentTemp);
      oled.setFont(fontMedium);
      oled.moveXY(offset+48, 14).write("C");
   }

   if (selected) {
      oled.drawRect(offset,  10, offset+57,  10+fontVeryLarge.height-9, WriteMode_Xor);
   }

   oled.setFont(fontLarge);
   oled.moveXY(offset,  35).write("P").setWidth(1).write(ch.getPreset()).
         write(ch.isTempModified()?"*:":" :").setWidth(3).write(ch.getUserTemperature());

   float power = ch.getPower();

   oled.setFont(fontSmall);
   oled.moveXY(offset,  50).write(ch.getTipName());

   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.moveXY(offset+30,  50).write(power).writeln('W');
   if (power>0) {
      int y = 59; //oled.getY();
      oled.drawRect(offset, y, offset-1+power,  y+4, WriteMode_Xor);
   }
}

/**
 * Display channel information on OLED
 */
void Display::displayChannels() {
   using namespace USBDM;

   static constexpr unsigned LEFT_OFFSET  = 1;
   static constexpr unsigned RIGHT_OFFSET = 1+(oled.WIDTH+1)/2;

   Channel &ch1 = channels[1];
   Channel &ch2 = channels[2];

   bool ch1Selected = channels.getSelectedChannelNumber() == 1;

   oled.clearDisplay();

   displayChannel(ch1, ch1Selected,  LEFT_OFFSET);
   displayChannel(ch2, !ch1Selected, RIGHT_OFFSET);

   oled.drawVerticalLine(RIGHT_OFFSET-3, 0, oled.HEIGHT-1, WriteMode_Write);

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

   int seconds = ms;

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
 * Display floating point menu item
 *
 * @param description   Description to display at top of screen.  May include a single newline to split into 2 lines.
 * @param value         Value to display.  This value is /1000 for scaling as a float value
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

   oled.moveXY(0, oled.getY()+2);
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
      oled.moveXY(1, oled.getY()+1);
      int menuY = oled.getY();
      oled.write(prefix);
      oled.writeln(item.name);
      if ((line+offset) == selection) {
         oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
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

   oled.clearDisplay();

   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());
   oled.moveXY(0, oled.getY()+2);

   Font &font = fontSmall;
   oled.setFont(font);
   oled.writeln(prompt);
   oled.moveXY(0, oled.getY()+2);

   for(int line=0; line<MIN_MENU_ENTRIES; line++) {
      if (options[line] == nullptr) {
         break;
      }
      int menuY = oled.getY();
      oled.moveXY(1, oled.getY()+1);
      oled.writeln(options[line]);
      if (line == selection) {
         oled.drawRect(0,  menuY, oled.WIDTH, menuY+font.height, WriteMode_Xor);
      }
   }

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display message and waits for user action
 *
 * @param[in] title     Title to display at top of screen
 * @param[in] message   Message to display
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

/**
 * Display calibration information for tip during calibration sequence
 *
 * @param title               Title to display
 * @param ch                  Channel being used for calibration
 * @param targetTemperature   Target temperature for this calibration stage
 */
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
   oled.write("Measured     ").write(round(ch.getCurrentTemperature())).writeln(" C");

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

/**
 * Display PID setting values
 *
 * @param tipname    Name of tip to display
 * @param selection  Index of selected item
 * @param stars      Array containing character to prefix menu items with
 * @param kp         Scaled PID Kp value
 * @param ki         Scaled PID Ki value
 * @param kd         Scaled PID Kd value
 * @param iLimit     Scaled PID I limit value
 *
 * @return
 */
EventType Display::displayPidSettings(const char *tipname, unsigned selection, char stars[4], int kp, int ki, int kd, int iLimit) {
   static constexpr float SCALE_FACTOR = TipSettings::FLOAT_SCALE_FACTOR;

   oled.clearDisplay();
   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(" PID Settings");
   oled.moveXY(0, oled.getY()+3);
   oled.write(" Tip: ").writeln(tipname);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY()+1);
   oled.moveXY(0, oled.getY()+5);

   Font &font = fontMedium;
   oled.setFont(font);
   oled.setFloatFormat(3, Padding_LeadingSpaces, 2);

   int menuY= oled.getY();
   oled.write(stars[0]).write("Kp      ").writeln(kp/SCALE_FACTOR);
   if (selection == 0) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
   }
   oled.moveXY(0, oled.getY()+2);
   menuY= oled.getY();
   oled.write(stars[1]).write("Ki      ").writeln(ki/SCALE_FACTOR);
   if (selection == 1) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
   }
   oled.moveXY(0, oled.getY()+2);
   menuY= oled.getY();
   oled.write(stars[2]).write("Kd      ").writeln(kd/SCALE_FACTOR);
   if (selection == 2) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
   }
   oled.moveXY(0, oled.getY()+2);
   menuY= oled.getY();
   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.write(stars[3]).write("I limit ").writeln(iLimit/SCALE_FACTOR);
   if (selection == 3) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+font.height-1, WriteMode_Xor);
   }

   oled.refreshImage();

   oled.resetFormat();

   return ev_None;
}
