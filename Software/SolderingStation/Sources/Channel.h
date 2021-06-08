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

struct Channel {
   int          currentTemperature;    // Measured temperature of tool
   int          targetTemperature;     // Desired temperature of tool
   unsigned     dutyCycle;             // Current duty-cycle for drive
   ChannelState state;                 // State of channel off/noTip etc
   unsigned     preset;                // Currently selected preset for the channel
   bool         modified;              // Indicates the targetTemperature has been altered from the preset value
};

#endif /* SOURCES_CHANNEL_H_ */
