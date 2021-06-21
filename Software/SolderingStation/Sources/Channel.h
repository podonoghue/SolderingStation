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
#include "Tips.h"

/// States for the channel
enum ChannelState {
   ch_off,        ///< Channel is off
   ch_noTip,      ///< No tip in iron
   ch_overload,   ///< Overload has been detected
   ch_fixedPower, ///< Tip supplied with constant power (T not maintained)
   ch_setback,    ///< Tip temperature has been lowered as idle for set-back period
   ch_active,     ///< Tip is being heated to user target temperature
};

/**
 * Class representing a channel
 */
class Channel : public DutyCycleCounter {

   using Controller = Pid;
//   using Controller = StepResponse;

public:

   /// Number of preset temperatures provided
   static constexpr unsigned NUM_PRESETS  = 3;

   /// Reference to non-volatile settings stored in Flash
   ChannelSettings  &nvSettings;

   /// Moving window average for tip temperature (thermocouple)
   ThermocoupleAverage tipTemperature;

   /// Moving window average for cold junction temperature (NTC resistor)
   ThermistorAverage coldJunctionTemperature;

   /// Average power - just for display purposes
   SimpleMovingAverage<5> power;

   friend class Control;

private:
   bool              tipPresent          = false;    ///< Indicates that the tip is present
   int               toolIdleTime        = 0;        ///< How long the tool has been idle
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
   Channel(ChannelSettings &settings) :
      DutyCycleCounter(100),
      nvSettings(settings){
      setUserTemperature(settings.presets[preset]);
      refreshPid();
   }

   /**
    * Check for a valid tip selection and re-assign if necessary
    *
    * @param defaultTip Tip use for channel if necessary
    */
   void checkTipSelected(TipSettings *defaultTip) {
      const TipSettings *ts = nvSettings.selectedTip;
      if ((ts == nullptr) || (ts->isFree())) {
         setTip(defaultTip);
      }
      else {
         // Refresh tip in case PID parameters changed
         setTip(nvSettings.selectedTip);
      }
   }

   void refreshPid() {
      const TipSettings *ts = nvSettings.selectedTip;

      controller.setControlParameters(
            ts->getKp(),
            ts->getKi(),
            ts->getKd(),
            ts->getILimit());
   }

   /**
    * Set selected tip for this channel
    *
    * @param index Index into tip settings table
    */
   void nextTip() {
      nvSettings.selectedTip = tips.getNextTip(nvSettings.selectedTip);
      refreshPid();
   }

   /**
    * Set selected tip for this channel
    *
    * @param index Index into tip settings table
    */
   void setTip(const TipSettings *tipSettings) {
      usbdm_assert(tipSettings != nullptr, "Illegal tip");
      nvSettings.selectedTip = tipSettings;
      refreshPid();
   }

   /**
    * Get selected tip for this channel
    *
    * @return Index into tip settings table
    */
   const TipSettings* getTip() const {
      return nvSettings.selectedTip;
   }

   const char *getTipName() {
      const TipSettings *ts = nvSettings.selectedTip;

      if (ts == nullptr) {
         return "----";
      }
      return ts->getTipName();
   }

   /**
    * Get human readable name of channel state
    *
    * @param state State to describe
    *
    * @return Name of state
    */
   static const char *getStateName(ChannelState state) {
      static const char *names[] = {
            "Off",
            "No Tip",
            "Over Ld",
            "Fixed Pwr",
            "Setback",
            "Active",
      };
      if (state >= (sizeof(names)/sizeof(names[0]))) {
         return "???";
      }
      return names[state];
   }

   /**
    * Set presence of tip.
    *
    * @param tipPresent
    */
   void setTipPresent(bool tipPresent) {
      if (nvSettings.selectedTip == nullptr) {
         setState(ch_off);
      }
      this->tipPresent = tipPresent;
   }

   /**
    * Gets state of channel modified by physical status of tip
    *
    * @return Status value
    */
   ChannelState getState() const {
      if (!tipPresent) {
         return ch_noTip;
      }
      return state;
   }

   /**
    * Get human readable name of channel state
    *
    * @return Name of state
    */
   const char *getStateName() const {
      return getStateName(getState());
   }

   /**
    * Set desired state of iron.
    * Clears physical status of iron but this will be updated when the iron is next polled.
    *
    * @param newState
    */
   void setState(ChannelState newState) {
      const TipSettings *ts = nvSettings.selectedTip;
      (void) ts;

      usbdm_assert((ts != nullptr) && !ts->isFree(), "No tip selected");
      state            = newState;
      refreshPid();
      controller.enable(isControlled());
      if (!isRunning()) {
         controller.setOutput(0);
         // For safety while debugging immediately turn off drive
         Ch1Drive::off();
         Ch2Drive::off();
      }
      applyPowerCorrection = false;
   }

   /**
    * Change overload state of channel
    *
    * @param value
    */
   void setOverload() {
      state = ch_overload;

      // Disable drive
      DutyCycleCounter::disable();
   }

   /**
    * Indicates if the channel is running i.e. in active, back-off or fixed-power states
    */
   bool isRunning(ChannelState state) const {
      return (state == ch_active) || (state == ch_setback)|| (state == ch_fixedPower) ;
   }

   /**
    * Indicates if the channel is running i.e. in active, back-off or fixed-power states
    */
   bool isRunning() const {
      return isRunning(getState());
   }

   /**
    * Indicates if the channel temperature is being controlled i.e. in active or back-off states
    */
   bool isControlled() const {
      ChannelState state = getState();
      return (state == ch_active) || (state == ch_setback) ;
   }

   /**
    * Gets the target temperature set by user.
    * This is independent of the state of the channel.
    *
    * @return User set temperature
    */
   int getUserTemperature() const {
      return targetTemperature;
   }

   /**
    * Gets the target temperature.
    * This is modified depending upon the state
    *
    * @return Target temperature
    */
   int getTargetTemperature() const {
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
   int getCurrentTemperature() const {
      return round(currentTemperature);
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

   float getPower() const {
      // Assume 24Vrms, 8.5 ohm element
      static constexpr float NOMINAL_MAX_POWER = (24 * 24)/8.5;
      float pwr = power.getAverage()*NOMINAL_MAX_POWER/100;
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
   unsigned getPreset() const {
      return preset+1;
   }

   /**
    * Get current preset temperature
    *
    * @return Temperature from preset
    */
   int getPresetTemperature() const {
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
   bool isTempModified() const {
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
