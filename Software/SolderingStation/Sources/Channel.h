/*
 * Channel.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CHANNEL_H_
#define SOURCES_CHANNEL_H_

#include <algorithm>
#include "hardware.h"
#include "Averaging.h"
#include "ChannelSettings.h"
#include "DutyCycleCounter.h"
#include "Pid.h"
#include "BangBang.h"
#include "StepResponse.h"

/// States for the channel
enum ChannelState {
   ch_off,        ///< Channel is off
   ch_noTip,      ///< No tip in iron
   ch_overload,   ///< Overload has been detected
   ch_setback,    ///< Tip temperature has been lowered as idle for set-back period
   ch_active,     ///< Tip is being heated to user target temperature
};

/**
 * Get human readable name of channel state
 *
 * @param state State to describe
 *
 * @return Name of state
 */
const char *getChannelStateName(ChannelState state);

/// Number of preset temperatures provided
static constexpr unsigned NUM_PRESETS  = 3;

/**
 * Class representing a channel
 */
class Channel : public DutyCycleCounter {

   using Controller = Pid;

public:

   /// Reference to non-volatile settings stored in Flash
   ChannelSettings  &nvSettings;

   /// Moving window average for tip temperature (thermocouple)
   ThermocoupleAverage tipTemperature;

   /// Moving window average for cold junction temperature (NTC resistor)
   ThermistorAverage coldJunctionTemperature;

   /// Average power - just for display purposes
   MovingWindowAverage<50> power;

   friend class Control;

private:
   bool              tipPresent          = false;    ///< Indicates that the tip is present
   int               toolIdleTime        = 0;        ///< How long the tool has been idle
   bool              overloadDetected    = false;    ///< Indicates an overload was detected
   float             currentTemperature  = 0;        ///< Measured temperature of tool
   ChannelState      state               = ch_off;   ///< State of channel ch_off/ch_standby/ch_backoff/ch_active
   int               targetTemperature   = 0;        ///< Desired temperature of tool
   unsigned          preset              = 1;        ///< Currently selected preset for the channel

   Controller        controller;                     ///< Loop controller

   bool              applyPowerCorrection = false;

public:
   /**
    * Constructor for channel
    *
    * @param settings Non-volatile channel settings to associate with this channel
    */
   Channel(ChannelSettings &settings, PidSettings &pidSettings) :
      DutyCycleCounter(100),
      nvSettings(settings),
      controller(pidSettings) {
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

      controller.enable(isRunning());
      applyPowerCorrection = false;
   }

   /**
    * Change overload state of channel
    *
    * @param value
    */
   void setOverload() {
      overloadDetected = true;

      // Disable drive
      DutyCycleCounter::disable();
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
    * Indicates if the channel is running i.e. in active or back-off states
    */
   bool isRunning() {
      ChannelState state = getState();
      return (state == ch_active) || (state == ch_setback) ;
   }

   /**
    * Gets the target temperature set by user.
    * This is independent of the state of the channel.
    *
    * @return User set temperature
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

         case ch_setback:
            return std::min((int)nvSettings.setbackTemperature, targetTemperature);

         default:
         case ch_off:
         case ch_noTip:
         case ch_overload:
            return 0;
      }
   }

   /**
    * Gets the stored current temperature.
    * This is updated periodically in the background.
    *
    * @return Current temperature
    */
   int getCurrentTemperature() {
      return currentTemperature;
   }

   /**
    * Update current temperature from internal averages
    * The PID controller is updated
    */
   void upDateCurrentTemperature() {
//      static int powerCorrection = 0;

      currentTemperature = tipTemperature.getTemperature()+coldJunctionTemperature.getTemperature();
//      if ((state == ch_active) && (currentTemperature > 150) && (controller.getElapsedTime()>10) &&
//          (abs(getTargetTemperature() - currentTemperature) < 10)) {
//         powerCorrection = std::min(30,(std::max(getPower()-8, 0))*13);
//      }
      tipPresent = tipTemperature.getAverage() < (USBDM::Adc0::getSingleEndedMaximum(ADC_RESOLUTION)-200);
      setDutyCycle(controller.newSample(getTargetTemperature(), currentTemperature));

      // Long term power average
      power.accumulate(getDutyCycle());
   }

   int getPower() {
      static constexpr int NOMINAL_MAX_POWER = (24 * 24)/8.5; // 24Vrms, 8.5 ohm element
      int pwr = power.getAverage()*NOMINAL_MAX_POWER/100;
      return pwr;
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
    * If in set-back state it will change to active state
    */
   void restartIdleTimer() {
      if (getState() == ch_setback) {
         setState(ch_active);
      }
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
    * Update current preset temperature from current user temperature.
    *
    * @note This is in non-volatile memory
    */
   void updatePresetTemperature() {
      nvSettings.presets[preset] = getUserTemperature();
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

#endif /* SOURCES_CHANNEL_H_ */
