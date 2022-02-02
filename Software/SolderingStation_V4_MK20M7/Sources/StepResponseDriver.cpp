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
bool StepResponseDriver::run(unsigned maxDrive) {

   static constexpr float TICK_INTERVAL    = 100_ms;

   static constexpr unsigned INITIAL_TIME  = round( 50.0/TICK_INTERVAL);
   static constexpr unsigned DRIVING_TIME  = round(600.0/TICK_INTERVAL);
   static constexpr unsigned COOLING_TIME  = round(100.0/TICK_INTERVAL);

   static constexpr unsigned REFRESH_TIME  = round(  1.0/TICK_INTERVAL);
   static constexpr unsigned REPORT_TIME   = round(  0.5/TICK_INTERVAL);

   static constexpr unsigned MIN_DRIVE = 0;

   channel.setState(ChannelState_fixedPower);

   enum {Step_Initial, Step_Driving, Step_Cooling, Step_Complete} state = Step_Initial;

   unsigned elapsedTime = 0;
   unsigned tickCount   = 0;
   int drive = MIN_DRIVE;
   bool success = true;

   console.write("Time,").write("Drive,").write("Temp: ").writeln(channel.getTipName());

   while ((state != Step_Complete) && success) {

      float currentTemp = channel.getCurrentTemperature();

      if ((tickCount%REPORT_TIME) == 0) {
         console.setWidth(4).setPadding(Padding_LeadingSpaces);
         console.setFloatFormat(1, Padding_LeadingSpaces, 3);
         console.write(elapsedTime*TICK_INTERVAL).write(", ").write(drive).write(", ").writeln(currentTemp);
         console.resetFormat();
      }
      if ((tickCount%REFRESH_TIME) == 0) {
         display.displayChannels();
      }

      elapsedTime++;
      tickCount++;

      switch(state) {

         case Step_Initial:
            if (tickCount >= INITIAL_TIME) {
               state     = Step_Driving;
               drive     = maxDrive;
               tickCount = 0;
            }
            break;

         case Step_Driving:
            if (tickCount >= DRIVING_TIME) {
               state     = Step_Cooling;
               drive     = MIN_DRIVE;
               tickCount = 0;
            }
            break;

         case Step_Cooling:
            if (tickCount >= COOLING_TIME) {
               state     = Step_Complete;
               tickCount = 0;
            }
            break;

         case Step_Complete:
         default:
            state = Step_Complete;
            drive = MIN_DRIVE;
            break;
      }
      channel.setDutyCycle(drive);
      success = (switchPolling.getEvent().type == ev_None) && (currentTemp<400);
      wait(TICK_INTERVAL);
   }

   channel.setDutyCycle(0);
   channel.setState(ChannelState_off);

   return success;
}
