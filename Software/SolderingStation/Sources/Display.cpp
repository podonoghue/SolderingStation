/*
 * Display.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "Display.h"

using namespace USBDM;

/**
 * Update OLED display
 *
 * @param ch1              Channel 1 properties
 * @param ch2              Channel 2 properties
 * @param selectedChannel  The currently selected channel
 */
void Display::displayTools(Channel &ch1, Channel &ch2, unsigned selectedChannel) {
   using namespace USBDM;

   static constexpr unsigned LEFT_OFFSET  = 1;
   static constexpr unsigned RIGHT_OFFSET = 1+(oled.WIDTH+1)/2;

   oled.clearDisplay();

   oled.setPadding(Padding_LeadingSpaces).setWidth(3);

   oled.setFont(fontVeryLarge);
   if ((ch1.state == ch_noTip) || (ch1.state == ch_overload)) {
      oled.moveXY(LEFT_OFFSET+4, 10).write("---");
   }
   else {
      oled.moveXY(LEFT_OFFSET, 10).write(ch1.currentTemperature);
      oled.setFont(fontMedium);
      oled.moveXY(LEFT_OFFSET+47, 15).write("C");
   }

   oled.setFont(fontVeryLarge);
   if ((ch2.state == ch_noTip) || (ch2.state == ch_overload)) {
      oled.moveXY(RIGHT_OFFSET+4, 10).write("---");
   }
   else {
      oled.moveXY(RIGHT_OFFSET, 10).write(ch2.currentTemperature);
      oled.setFont(fontMedium);
      oled.moveXY(RIGHT_OFFSET+47, 15).write("C");
   }

   switch (selectedChannel) {
      case 1:
         oled.drawRect(LEFT_OFFSET-1,  12, LEFT_OFFSET+56,  12+23, WriteMode_Xor);
         break;
      case 2:
         oled.drawRect(RIGHT_OFFSET-1,  12, RIGHT_OFFSET+56,  12+23, WriteMode_Xor);
         break;
   }

   oled.setFont(fontMedium);
   oled.moveXY(LEFT_OFFSET, 0).write(getChannelStateName(ch1.state));
   oled.moveXY(RIGHT_OFFSET, 0).write(getChannelStateName(ch2.state));

   oled.setFont(fontLarge);
   oled.moveXY(LEFT_OFFSET,  40).write("P").setWidth(1).write(ch1.preset+1).
         write(ch1.modified?"*:":" :").setWidth(3).write(ch1.targetTemperature);
   oled.moveXY(RIGHT_OFFSET, 40).write("P").setWidth(1).write(ch2.preset+1).
         write(ch2.modified?"*:":" :").setWidth(3).write(ch2.targetTemperature);

   oled.setFont(fontMedium).setWidth(2);
   oled.moveXY(LEFT_OFFSET,  56).write(ch1.dutyCycle).write('%');
   oled.moveXY(RIGHT_OFFSET, 56).write(ch2.dutyCycle).write('%');
   oled.drawRect(LEFT_OFFSET,  55, LEFT_OFFSET+ch1.dutyCycle/2,  63, WriteMode_Xor);
   oled.drawRect(RIGHT_OFFSET, 55, RIGHT_OFFSET+ch2.dutyCycle/2, 63, WriteMode_Xor);

   oled.drawVerticalLine(RIGHT_OFFSET-4, 0, oled.HEIGHT-1, WriteMode_Write);

   oled.refreshImage();

   oled.resetFormat();
}

void Display::displayTimeMenuItem(unsigned channel, const char *name, unsigned seconds) {
   oled.clearDisplay();

   oled.setFont(fontLarge);
   oled.moveXY(0, 0).writeln(name).write("CH ").write(channel);

   oled.setFont(fontVeryLarge);
   oled.setPadding(Padding_LeadingSpaces).setWidth(4);
   oled.moveXY(15, 30).write(seconds).write("s");

   oled.refreshImage();

   oled.resetFormat();
}

void Display::displayTemperatureMenuItem(unsigned channel, const char *name, unsigned temperature) {
   oled.clearDisplay();

   oled.setFont(fontLarge);
   oled.moveXY(0, 0).writeln(name).write("CH ").write(channel);

   oled.setFont(fontVeryLarge);
   oled.setPadding(Padding_LeadingSpaces).setWidth(4);
   oled.moveXY(15, 30).write(temperature).write("C");

   oled.refreshImage();

   oled.resetFormat();
}


