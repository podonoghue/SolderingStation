/*
 * Channel.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CHANNEL_H_
#define SOURCES_CHANNEL_H_

#include <algorithm>

enum ChannelState {ch_off, ch_noTip, ch_overload, ch_backoff, ch_active};

const char *getChannelStateName(ChannelState state);

static constexpr unsigned NUM_PRESETS  = 3;

class Channel {
public:
   /// Temperature to use when the iron has been idle for a while
   static constexpr int BACK_OFF_TEMPERATURE = 200;

   int          currentTemperature;    // Measured temperature of tool
   unsigned     dutyCycle;             // Current duty-cycle for drive
   unsigned     preset;                // Currently selected preset for the channel
   bool         modified;              // Indicates the targetTemperature has been altered from the preset value
   bool         overloadDetected;
   bool         tipPresent;

private:
   ChannelState state;                 // State of channel ch_off/ch_standby/ch_backoff/ch_active
   int          targetTemperature;     // Desired temperature of tool

public:
   /**
    * Gets state of iron modified by physical status of tip
    *
    * @return Status value
    */
   ChannelState getState() {
      if (overloadDetected) {
         return ch_overload;
      }
      if (!tipPresent) {
         return ch_noTip;
      }
      return state;
   }

   /**
    * Set desired state of iron.
    * Clears physical status of iron but this will be updated when the iron is polled.
    *
    * @param newState
    */
   void setState(ChannelState newState) {
      state            = newState;
      overloadDetected = false;
   }

   void setOverload(bool value) {
      overloadDetected = value;
   }

   void setTipPresent(bool value) {
      tipPresent = value;
   }

   bool isRunning() {
      ChannelState state = getState();
      return (state == ch_active) || (state == ch_backoff) ;
   }

   /**
    * Gets the target temperature.
    * This is modified depending upon the state
    *
    * @return Target temperature
    */
   int getTargetTemperature() {
      switch(getState()) {
         case ch_active:
            return targetTemperature;

         default:
         case ch_off:
         case ch_noTip:
         case ch_overload:
            return 0;

         case ch_backoff:
            return std::min(BACK_OFF_TEMPERATURE, targetTemperature);
      }
   }

   /**
    * Sets the target temperature.
    *
    * @param targetTemp The temperature to set.
    */
   void setTargetTemperature(int targetTemp) {
      targetTemperature = targetTemp;
   }
};

#endif /* SOURCES_CHANNEL_H_ */
