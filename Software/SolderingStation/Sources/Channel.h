/*
 * Channel.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CHANNEL_H_
#define SOURCES_CHANNEL_H_

enum ChannelState {ch_off, ch_noTip, ch_overload, ch_standby, ch_backoff, ch_active};

const char *getChannelStateName(ChannelState state);

static constexpr unsigned NUM_PRESETS  = 3;

class Channel {
public:
   int          currentTemperature;    // Measured temperature of tool
   int          targetTemperature;     // Desired temperature of tool
   unsigned     dutyCycle;             // Current duty-cycle for drive
   unsigned     preset;                // Currently selected preset for the channel
   bool         modified;              // Indicates the targetTemperature has been altered from the preset value
   bool         overloadDetected;
   bool         tipPresent;

private:
   ChannelState state;                 // State of channel ch_off/ch_standby/ch_backoff/ch_active

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
      return (state == ch_active) || (state == ch_backoff) || (state == ch_standby);
   }
};

#endif /* SOURCES_CHANNEL_H_ */
