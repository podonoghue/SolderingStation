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

FontArraySubset<FontVeryLarge,
' ', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'l', 'o', 'w', '-', '.', '*', 'm', 's', 'O', 'f'>
fontVeryLargeReduced;

/**
 * Display information about one channel on OLED.
 * Used to draw left and right halves of screen
 *
 * @param ch         Channel to display
 * @param offset     Y offset for display
 */
void Display::displayChannelStatus(Channel &, unsigned ) {
   //   using namespace USBDM;
   //
   //   oled.moveXY(0, offset);
   //   oled.setFont(fontMedium);
   //
   //   float Tc = ch.getThermisterTemperature();
   //   float Tt = ch.getThermocoupleTemperature();
   //
   //   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   ////   oled.writeln("Rc ", ch.getThermisterResistance()/1000, "k,");
   //   oled.writeln(Tc, "C");
   //
   //   oled.setFloatFormat(2, Padding_LeadingSpaces, 0);
   ////   oled.write("Vt ", round(ch.getThermocoupleVoltage()*100000)/100, "mV,");
   //   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   //   oled.writeln(Tt, "C");
   //   oled.writeln("T = ", Tc+Tt, "C");
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
   oled.write("Chip ", control.getChipTemperature(), "C");

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

   oled.setFont(fontVeryLargeReduced);
   if ((ch.getState() == ChannelState_noTip) || (ch.getState() == ChannelState_overload)) {
      oled.moveXY(offset+6, 8).write("---");
   }
   else {
      int currentTemp = round(ch.getCurrentTemperature());
      if (currentTemp>999) {
         currentTemp = 999;
      }
      oled.moveXY(offset+2, 8);
      if (currentTemp < 40) {
         oled.write("low");
      }
      else {
         oled.write(currentTemp);
         oled.setFont(fontMedium);
         oled.moveXY(offset+48, 14).write("C");
      }
   }

   if (selected) {
      oled.drawRect(offset,  10, offset+57,  10+fontVeryLargeReduced.HEIGHT-9, WriteMode_Xor);
   }

   oled.setFont(fontLarge);
   if (ch.getState()==ChannelState_setback) {
      // If in setback mode display target temperature instead of active preset
      oled.moveXY(offset,  35).write("SB :").setWidth(3).write(ch.getTargetTemperature());
   }
   else {
      oled.moveXY(offset,  35).write("P").setWidth(1).write(ch.getPreset()).
            write(ch.isTempModified()?"*:":" :").setWidth(3).write(ch.getUserTemperature());
   }
   float power = ch.measurement->getPower();

   oled.setFont(fontSmall);
   oled.moveXY(offset,  50).write(ch.getTipName());

   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.moveXY(offset+35,  50).write((int)round(power), 'W');

   float percentagePower = ((oled.WIDTH/2)-3)*(ch.measurement->getPercentagePower()/100);
#if 0
   static constexpr int BG_TOP    = 58;
   static constexpr int BG_BOTTOM = oled.HEIGHT-1;
   oled.drawHorizontalLine(0, oled.WIDTH, BG_TOP,    WriteMode_Or);
   oled.drawHorizontalLine(0, oled.WIDTH, BG_BOTTOM, WriteMode_Or);
   oled.drawVerticalLine(0,            BG_TOP, BG_BOTTOM, WriteMode_Or);
   oled.drawVerticalLine(oled.WIDTH-1, BG_TOP, BG_BOTTOM, WriteMode_Or);
   if (percentagePower>1) {
      oled.drawRect(offset, BG_TOP+1, offset-1+percentagePower,  BG_BOTTOM-1, WriteMode_Xor);
   }
#else
   static constexpr int BG_TOP    = 58;
   static constexpr int BG_BOTTOM = oled.HEIGHT-1;
   if (percentagePower>1) {
      oled.drawRect(offset, BG_TOP, offset-1+percentagePower,  BG_BOTTOM, WriteMode_Xor);
   }
#endif
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

   oled.enable(true);
}

/**
 * Display time menu item
 *
 * @param description      Description to display at top of screen.  May include a single newline to split into 2 lines.
 * @param seconds          Time in seconds to display
 * @param modified         Indicates item has been modified since saving - adds indicator to display
 */
void Display::displayTimeMenuItem(const char *description, unsigned seconds, bool modified) {
   oled.clearDisplay();

   oled.setFont(fontLarge);
   oled.moveXY(0, 0).writeln(description);

   oled.setFont(fontVeryLargeReduced);
   oled.moveXY(0, 30).write(modified?'*':' ');

   if (seconds>0) {
      if (seconds>=60) {
         oled.setPadding(Padding_LeadingSpaces).setWidth(2).write(seconds/60, "m");
      }
      else {
         oled.write("   ");
      }
      seconds %= 60;
      oled.setPadding(Padding_LeadingZeroes).setWidth(2).write(seconds, "s");
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

   oled.setFont(fontVeryLargeReduced);
   oled.moveXY(10, 30).write(modified?'*':' ');
   oled.write(value/1000, '.');
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

   oled.setFont(fontVeryLargeReduced);
   oled.setPadding(Padding_LeadingSpaces).setWidth(4);
   oled.moveXY(0, 30).write(modified?'*':' ', temperature);
   oled.moveXY(80, 35).setFont(fontMedium).write("C");

   //   oled.moveXY(28, 55).setFont(fontSmall).write(" <-           ->");

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display a menu list with selected item
 *
 * @param [in]     title         Title to display at top of screen
 * @param [in]     items         Array of menu items.
 * @param [in]     modifiersUsed Modifiers that may be applied to items (for sizing)
 * @param [inout]  offset        Offset into list for display i.e. first item on visible menu.
 * @param [in]     selection     Selected item index
 */
void Display::displayMenuList(const char *title, MenuItem const items[], unsigned modifiersUsed, BoundedMenuState &selection) {

   oled.clearDisplay();

   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());

   oled.moveXY(0, oled.getY()+2);
   oled.setFont(fontSmall);

   for(int menuLine=0; menuLine<=min(NUM_MENU_ENTRIES-1, selection.getMax()); menuLine++) {
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
      int itemIndex = selection.getLineIndex(menuLine);
      auto item = items[itemIndex];
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
      if (itemIndex == selection) {
         // Highlight selected item
         oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+fontSmall.HEIGHT-1, WriteMode_Xor);
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

   oled.setFont(fontSmall);
   oled.writeln(prompt);
   oled.moveXY(0, oled.getY()+2);

   for(int line=0; line<NUM_MENU_ENTRIES; line++) {
      if (options[line] == nullptr) {
         break;
      }
      int menuY = oled.getY();
      oled.moveXY(1, oled.getY()+1);
      oled.writeln(options[line]);
      if (line == selection) {
         oled.drawRect(0,  menuY, oled.WIDTH, menuY+fontSmall.HEIGHT, WriteMode_Xor);
      }
   }

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Display message and return immediately
 *
 * @param[in] title     Title to display at top of screen
 * @param[in] message   Message to display
 */
void Display::showMessage(const char *title, const char *message) {
   oled.clearDisplay();

   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY());

   oled.moveXY(0, oled.getY()+2);
   oled.setFont(fontSmall);
   oled.writeln(message);

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
   showMessage(title, message);

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
   oled.writeln("Target ----> ", targetTemperature, " C");

   oled.moveXY(0, oled.getY()+3);
   oled.writeln("Measured     ", (int)round(ch.getCurrentTemperature()), " C");

   oled.moveXY(0, oled.getY()+3);
   oled.writeln("Controlled   ", ch.getUserTemperature(), " C");

   oled.moveXY(0, oled.getY()+5);
   ch.measurement->reportCalibrationValues(oled, true);

   //   oled.write(" Power ", ch.getPower());
   //   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   //   oled.write("Vt ", 1000*ch.tipTemperature.getVoltage(), "mV");

   //   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   //   oled.write(", Tc ", ch.coldJunctionTemperature.getTemperature(), "C");

   oled.refreshImage();

   oled.resetFormat();
}

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
 */
void Display::displayPidSettings(const char *tipname, unsigned selection, char stars[4], int kp, int ki, int kd, int iLimit) {
   static constexpr float SCALE_FACTOR = TipSettings::FLOAT_SCALE_FACTOR;

   oled.clearDisplay();
   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(" PID Settings");
   oled.moveXY(0, oled.getY()+3);
   oled.writeln(" Tip: ", tipname);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY()+1);
   oled.moveXY(0, oled.getY()+5);

   oled.setFont(fontMedium);
   oled.setFloatFormat(3, Padding_LeadingSpaces, 2);

   int menuY= oled.getY();
   oled.writeln(stars[0], "Kp      ", kp/SCALE_FACTOR);
   if (selection == 0) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+fontMedium.HEIGHT-1, WriteMode_Xor);
   }
   oled.moveXY(0, oled.getY()+2);
   menuY= oled.getY();
   oled.writeln(stars[1], "Ki      ", ki/SCALE_FACTOR);
   if (selection == 1) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+fontMedium.HEIGHT-1, WriteMode_Xor);
   }
   oled.moveXY(0, oled.getY()+2);
   menuY= oled.getY();
   oled.writeln(stars[2], "Kd      ", kd/SCALE_FACTOR);
   if (selection == 2) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+fontMedium.HEIGHT-1, WriteMode_Xor);
   }
   oled.moveXY(0, oled.getY()+2);
   menuY= oled.getY();
   oled.setFloatFormat(1, Padding_LeadingSpaces, 2);
   oled.writeln(stars[3], "I limit ", iLimit/SCALE_FACTOR);
   if (selection == 3) {
      oled.drawRect(0,  menuY-1, oled.WIDTH, menuY+fontMedium.HEIGHT-1, WriteMode_Xor);
   }

   oled.refreshImage();

   oled.resetFormat();
}

/**
 * Channel status display - for debug
 *
 * @param title      Title to display at top of screen
 * @param ch         Channel to report on
 * @param dutyCycle  Duty cycle to display
 */
void Display::displayHeater(const char *title, Channel &ch, unsigned dutyCycle) {
   float power = ch.measurement->getPower();
   float chipTemp = control.getChipTemperature();

   oled.clearDisplay();
   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln(title);
   oled.moveXY(0, oled.getY()+2);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY()+1);
   oled.moveXY(0, oled.getY()+3);
   oled.setFont(fontSmall);
   oled.setFloatFormat(1);
   oled.writeln("Chip = ", chipTemp, " C");
   oled.writeln("DutyCyle=", dutyCycle, " (", power, " W)");
   oled.moveXY(0, oled.getY()+3);
   ch.measurement->reportCalibrationValues(oled);
   oled.refreshImage();
   oled.resetFormat();
}

/**
 * Report changes between old and new calibration values
 *
 * @param oldTs    Original values
 * @param newTs    Changed values
 *
 * @return True  - Confirmed (Short press)
 * @return False - Cancelled (Press and hold)
 */
bool Display::reportSettingsChange(const TipSettings &oldTs, const TipSettings &newTs) {
   oled.clearDisplay();
   oled.setFont(fontMedium);
   oled.moveXY(0, 0);
   oled.writeln("Calibration");
   oled.moveXY(0, oled.getY()+2);
   oled.drawHorizontalLine(0, oled.WIDTH, oled.getY()+1);
   oled.moveXY(0, oled.getY()+3);
   oled.setFont(fontSmall);
   oled.setFloatFormat(0);

   oled.writeln("   Old       New");
   oled.moveXY(0, oled.getY()+3);
   for (CalibrationIndex index=CalibrationIndex_250; index<=CalibrationIndex_400; ++index) {
      oled.setFloatFormat(0).write("(", oldTs.getCalibrationTempValue(index), ",")
                .setFloatFormat(1, Padding_LeadingSpaces, 2).write(oldTs.getCalibrationMeasurementValue(index), ")(")
                .setFloatFormat(0).write(newTs.getCalibrationTempValue(index), ",")
                .setFloatFormat(1, Padding_LeadingSpaces, 2).writeln(newTs.getCalibrationMeasurementValue(index), ")");
   }
   oled.moveXY(0, oled.getY()+3);
   oled.writeln("Long press to discard");

   oled.refreshImage();
   oled.resetFormat();

   Event event;
   while (!event.isSelHold() && !event.isSelRelease()) {
      event = switchPolling.getEvent();
   }

   return ((event.type == ev_SelRelease) || (event.type == ev_QuadRelease));
}
