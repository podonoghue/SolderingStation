/*
 * StepResponseDriver.cpp
 *
 *  Created on: 22 Jun. 2021
 *      Author: peter
 */

#include "hardware.h"

#include "StepResponseDriver.h"
#include "Channel.h"
#include "SwitchPolling.h"

using namespace USBDM;

/**
 * Constructor
 *
 * @param channel  Associated channel
 */
StepResponseDriver::StepResponseDriver(Channel &channel) : channel(channel) {
}

/**
 * Run step sequence
 *
 * @return true  Completed successfully
 * @return false Failed
 */
bool StepResponseDriver::run() {

   static constexpr unsigned INITIAL_TIME = 50;
   static constexpr unsigned DRIVING_TIME = 600;
   static constexpr unsigned COOLING_TIME = 100;

   static constexpr unsigned MIN_DRIVE = 0;
   static constexpr unsigned MAX_DRIVE = 5;

   channel.setState(ch_fixedPower);

   enum {Step_Initial, Step_Driving, Step_Cooling, Step_Complete} state = Step_Initial;

   unsigned tickCount = 0;
   int drive = MIN_DRIVE;
   bool success = true;

   console.write("Time,").write("Drive,").write("Temp: ").writeln(channel.getTipName());

   while ((state != Step_Complete) && success) {

      console.setWidth(4).setPadding(Padding_LeadingSpaces);
      console.setFloatFormat(1, Padding_LeadingSpaces, 3);
      console.write(tickCount).write(", ").write(drive).write(", ").writeln(channel.getCurrentTemperature());
      console.resetFormat();

      tickCount++;
      switch(state) {
         case Step_Initial:
            if (tickCount >= INITIAL_TIME) {
               state = Step_Driving;
               drive = MAX_DRIVE;
            }
            break;
         case Step_Driving:
            if (tickCount >= INITIAL_TIME+DRIVING_TIME) {
               state = Step_Cooling;
               drive = MIN_DRIVE;
            }
            break;
         case Step_Cooling:
            if (tickCount >= INITIAL_TIME+DRIVING_TIME+COOLING_TIME) {
               state = Step_Complete;
            }
            break;
         default:
            state = Step_Complete;
            drive = MIN_DRIVE;
            break;
      }
      channel.setDutyCycle(drive);
      success = (switchPolling.getEvent().type != ev_QuadHold);
      display.displayChannels();
      waitMS(1000);
   }

   channel.setDutyCycle(0);
   channel.setState(ch_off);

   return success;
}
