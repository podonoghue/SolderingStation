/*
 * Channel.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CHANNEL_H_
#define SOURCES_CHANNEL_H_

#include <algorithm>
#include <error.h>
#include "flash.h"

enum ChannelState {ch_off, ch_noTip, ch_overload, ch_backoff, ch_active};

const char *getChannelStateName(ChannelState state);

static constexpr unsigned NUM_PRESETS  = 3;

constexpr unsigned IDLE_MAX_TIME      =  5*60*1000; // 5 minutes in ms
constexpr unsigned LONGIDLE_MAX_TIME  = 20*60*1000; // 20 minutes in ms

class ChannelSettings {

public:
   /// Preset temperatures
   USBDM::Nonvolatile<int>    presets[3];

   // Back off temperature for idle iron
   USBDM::Nonvolatile<int>    backOffTemperature;

   // Idle time delay until backing off tip temperature ms
   USBDM::Nonvolatile<int>    backOffTime;

   // Idle time delay until turning off idle iron ms
   USBDM::Nonvolatile<int>    safetyOffTime;
};

class Channel {

public:
   /// Temperature to use when the iron has been idle for a while
   static constexpr int BACK_OFF_TEMPERATURE = 200;

   unsigned          dutyCycle           = 0;     // Current duty-cycle for drive
   bool              overloadDetected    = false; // Indicates an overload was detected
   bool              tipPresent          = false; // Indicates that the tip is present
   int               toolIdleTime        = 0;     // How long the tool has been idle

   ChannelSettings  &nvSettings;                  // Non-volatile settings stored in Flash


private:
   int               currentTemperature  = 0;        // Measured temperature of tool
   ChannelState      state               = ch_off;   // State of channel ch_off/ch_standby/ch_backoff/ch_active
   int               targetTemperature   = 0;        // Desired temperature of tool
   unsigned          preset              = 1;        // Currently selected preset for the channel

public:
   Channel(ChannelSettings &settings) : nvSettings(settings) {
      setUserTemperature(settings.presets[preset]);
   }

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

   /**
    * Change overload state of channel
    *
    * @param value
    */
   void setOverload(bool value) {
      overloadDetected = value;
   }

   /**
    * Change tip present state of channel
    *
    * @param value
    */
   void setTipPresent(bool value) {
      tipPresent = value;
   }

   /**
    * Indicates if the channel is running i.e. in active or backoff states
    */
   bool isRunning() {
      ChannelState state = getState();
      return (state == ch_active) || (state == ch_backoff) ;
   }

   /**
    * Gets the target temperature set.
    * This is independent of the state
    *
    * @return Set temperature
    */
   int getUserTemperature() {
      return targetTemperature;
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
    * Gets the stored current temperature.
    *
    * @return Current temperature
    */
   int getCurrentTemperature() {
      return currentTemperature;
   }

   /**
    * Set current temperature
    *
    * @param temperature Current temperature to set
    */
   void setCurrentTemperature(int temperature) {
      currentTemperature = temperature;
   }

   /**
    * Sets the user target temperature.
    *
    * @param targetTemp The temperature to set.
    */
   void setUserTemperature(int targetTemp) {
      targetTemperature = targetTemp;
   }

   /**
    * Restart tool idle time counter
    */
   void restartIdleTimer() {
      toolIdleTime = 0;
   }

   /**
    * Increment currently selected preset
    */
   void incrementPreset() {
      preset++;
      if (preset>=NUM_PRESETS) {
         preset = 0;
      }
   }

   /**
    * Get currently selected preset (1..NUM_PRESETS)
    */
   unsigned getPreset() {
      return preset+1;
   }

   /**
    * Get current preset temperature
    *
    * @return Temperature from preset
    */
   int getPresetTemperature() {
      return nvSettings.presets[preset];
   }

   /**
    * Update current preset temperature from current user temperature
    */
   void updatePresetTemperature() {
      if (isTempModified()) {
         nvSettings.presets[preset] = getUserTemperature();
      }
   }

   /**
    * Indicates if the temperature has been changed since a preset was selected.
    *
    * @return true if temperature has been changed
    */
   bool isTempModified() {
      return targetTemperature != nvSettings.presets[preset];
   }

   /**
    * Increment tool idle time.
    * Protected from roll-over.
    *
    * @param milliseconds Amount to increment the counter by
    *
    * @return idle time
    */
   unsigned incrementIdleTime(int milliseconds) {
      if (toolIdleTime<(INT_MAX-milliseconds)) {
         toolIdleTime += milliseconds;
      }
      return toolIdleTime;
   }
};

extern ChannelSettings ch1Settings;
extern ChannelSettings ch2Settings;

/**
 * Class representing a channel
 * This includes settings and channel state
 */
class Channels {

public:

   /// Number of channels
   static constexpr unsigned NUM_CHANNELS = 2;

   /// Channel1 state information
   Channel  channel1 = Channel(ch1Settings);

   /// Channel2 state information
   Channel  channel2 = Channel(ch2Settings);

   /// Currently selected channel for front panel controls
   unsigned selectedChannel = 0;

   /**
    * Get channel by channel number
    *
    * @param channelNumber
    *
    * @return Reference to indicate channel
    */
   Channel &operator[](int channelNumber) {
      usbdm_assert((channelNumber == 1)||(channelNumber == 2), "Illegal channel");

      return (channelNumber==1)?channel1:channel2;
   }

   /**
    * Get the number of the selected channel
    *
    * @return Channel number (zero indicates no currently selected channel)
    */
   unsigned getSelectedChannelNumber() {
      return selectedChannel;
   }

   /**
    * Get currently selected channel for front panel controls
    *
    * @return Reference to currently selected channel
    */
   Channel &getSelectedChannel() {
      return this->operator[](selectedChannel);
   }

   /**
    * Set currently selected channel for front panel controls
    *
    * @param channel Channel to make selected
    */
   void setSelectedChannel(unsigned channel) {
      selectedChannel = channel;
   }
};

/// The channel information
extern Channels channels;

#endif /* SOURCES_CHANNEL_H_ */
