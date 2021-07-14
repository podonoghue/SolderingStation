/*
 * Channel.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CHANNEL_H_
#define SOURCES_CHANNEL_H_

#include "BangBangController.h"
#include "hardware.h"
#include "Averaging.h"
#include "Measurement.h"
#include "ChannelSettings.h"
#include "DutyCycleCounter.h"
#include "Controller.h"
#include "Tips.h"
#include "T12.h"
#include "WellerWT50.h"

class StepResponseDriver;

/// States for the channel
enum ChannelState {
   ch_off,        ///< Channel is off
   ch_noTip,      ///< No tip in iron
   ch_overload,   ///< Overload has been detected
   ch_fixedPower, ///< Tip supplied with constant power (T not maintained)
   ch_setback,    ///< Tip temperature has been lowered as idle for set-back period
   ch_active,     ///< Tip is being heated to user target temperature
};

enum IronType {
   IronType_Weller,
   IronType_T12,
};

/**
 * Class representing a channel
 */
class Channel : public DutyCycleCounter {

   friend StepResponseDriver;

private:
   bool              tipPresent          = false;    ///< Indicates that the tip is present
   int               toolIdleTime        = 0;        ///< How long the tool has been idle
   float             currentTemperature  = 0;        ///< Measured temperature of tool
   ChannelState      state               = ch_off;   ///< State of channel ch_off/ch_standby/ch_backoff/ch_active
   int               targetTemperature   = 0;        ///< Desired temperature of tool
   unsigned          preset              = 1;        ///< Currently selected preset for the channel

   Controller       &controller;                     ///< Loop controller
   Weller_WT50       wellerMeasurement;
   T12               t12Measurement;

public:

   /// Number of preset temperatures provided
   static constexpr unsigned NUM_PRESETS  = 3;

   /// Reference to non-volatile settings stored in Flash
   ChannelSettings  &nvSettings;

   /// Measurement class
   Measurement *measurement = nullptr;

   /// Average power - just for display purposes
   SimpleMovingAverage<5> power;

   friend class Control;

   virtual void ledWrite(bool)   = 0;
   virtual void driveWrite(bool) = 0;
   virtual bool driveReadState() = 0;

   /**
    * Constructor for channel
    *
    * @param[in] settings        Non-volatile channel settings to associate with this channel
    * @param[in] controller      Temperature controller e.g. PID
    * @param[in] tipTemperature  Tip temperature averaging etc.
    * @param[in] coldJunction    Cold junction averaging etc.
    */
   Channel(ChannelSettings &settings, Controller &controller) :
      DutyCycleCounter(100),
      controller(controller),
      nvSettings(settings) {
      setIronType(IronType_Weller);
      setUserTemperature(settings.presets[preset]);
      refreshControllerParameters();
   }

   virtual ~Channel() {}

   /**
    * Set type of soldering iron being used
    *
    * @param ironType
    */
   void setIronType(IronType ironType) {

      switch(ironType) {
         case IronType_T12:
            measurement = &t12Measurement;
            break;
         case IronType_Weller:
            measurement = &wellerMeasurement;
            break;
         default:
            usbdm_assert(false, "Illegal iron type");
      }
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

   /**
    * Update controller parameters from currently selected tip
    */
   void refreshControllerParameters() {
      const TipSettings *ts = nvSettings.selectedTip;

      controller.setControlParameters(ts);
   }

   /**
    * Set selected tip for this channel
    *
    * @param index Index into tip settings table
    */
   void changeTip(int delta) {
      nvSettings.selectedTip = tips.changeTip(nvSettings.selectedTip, delta);
      refreshControllerParameters();
   }

   /**
    * Set selected tip for this channel
    *
    * @param index Index into tip settings table
    */
   void setTip(const TipSettings *tipSettings) {
      usbdm_assert(tipSettings != nullptr, "Illegal tip");
      nvSettings.selectedTip = tipSettings;
      refreshControllerParameters();
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
            "Fixed",
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

      state = newState;

      refreshControllerParameters();

      controller.enable(isControlled());
      ledWrite(isRunning());

      if (!isRunning()) {
         controller.setOutput(0);

         // For safety while debugging immediately turn off drive
         driveWrite(false);
      }
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
   float getCurrentTemperature() const {
      return currentTemperature;
   }

   /**
    * Update current temperature from internal averages
    * The PID controller is updated
    */
   void upDateCurrentTemperature() {
      currentTemperature = measurement->getTemperature();
      setTipPresent(currentTemperature < 600);
      // Power average
      power.accumulate(getDutyCycle());
   }

   /**
    * Update current temperature from internal averages
    * The controller is updated and used to determine drive
    */
   void upDatePid() {
      if (state != ch_fixedPower) {
         setDutyCycle(controller.newSample(getTargetTemperature(), currentTemperature));
      }
   }

//   /**
//    * Get Thermistor resistance
//    *
//    * @return Resistance in ohms
//    */
//   float getThermisterResistance() {
//      return coldJunctionTemperature.getResistance();
//   }

//   /**
//    * Get Thermistor temperature
//    *
//    * @return Temperature in C
//    */
//   float getThermisterTemperature() {
//      return coldJunctionTemperature.getTemperature();
//   }

//   /**
//    * Get thermocouple voltage
//    *
//    * @return Voltage in volts
//    */
//   float getThermocoupleVoltage() {
//      return tipTemperature.getVoltage();
//   }

//   /**
//    * Get thermocouple temperature
//    *
//    * @return Temperature in C
//    */
//   float getThermocoupleTemperature() {
//      return tipTemperature.getTemperature();
//   }

   float getPower() const {
      // Assume 24Vrms
      float NOMINAL_MAX_POWER = (24 * 24)/measurement->getHeaterResistance();
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
    * Increment currently selected preset.
    * Set temperature is updated
    * Restart timer is cleared
    */
   void nextPreset() {
      preset++;
      if (preset>=NUM_PRESETS) {
         preset = 0;
      }
      setUserTemperature(getPresetTemperature());
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
    * Update drive based on PWM state
    */
   /**
    *
    * @param borrowCycle Whether to borrow a cycle for sampling
    */
   void updateDrive() {
      advance();
      driveWrite(isOn());
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
    * Restart tool idle time counter.
    * If in set-back state it will change to active state.
    */
   void restartIdleTimer() {
      if (getState() == ch_setback) {
         setState(ch_active);
      }
      toolIdleTime = 0;
   }

   /**
    * Increment tool idle time.
    * Protected from roll-over.
    * Does set-back when idle or off when idle for long time.
    *
    * @param milliseconds Amount to increment the idle time by
    *
    *
    */
   void incrementIdleTime(int milliseconds) {

      if (toolIdleTime<(INT_MAX-milliseconds)) {
         toolIdleTime += milliseconds;
      }

      if ((state == ch_active) && (nvSettings.setbackTime > 0) && (toolIdleTime >= nvSettings.setbackTime*1000)) {
         // Idle for a short while while active
         setState(ch_setback);
      }
      if (isRunning() && (nvSettings.safetyOffTime > 0) && (toolIdleTime >= nvSettings.safetyOffTime*1000)) {
         // Idle for a long while running
         setState(ch_off);
      }
   }

   void report() {
//      USBDM::console.setFloatFormat(2, USBDM::Padding_LeadingSpaces, 2);
//      float tipV   = 1000*tipTemperature.getVoltage();
//      float tipT   = tipTemperature.getTemperature();
//      float coldT  = coldJunctionTemperature.getTemperature();
//
//      USBDM::console.
//      write("Tip = ").write(tipT+coldT).
//      write(" (").write(tipT).write("+").write(+coldT).
//      write("),(").write(tipV).write(" mV").
//      write(",").write(coldJunctionTemperature.getResistance()).write(" ohms)").
//      write(" ").write(tipV).write(" ").write(coldT).write(" ").
//      //         write(" C, Ch1 Cold = ").write(ch1ColdJunctionTemperature.getConvertedValue()).
//      //         write(" C, Ch2 Tip  = ").write(ch2TipTemperature.getConvertedValue()+ch2ColdJunctionTemperature.getConvertedValue()).
//      //         write(" C, Ch2 Cold = ").write(ch2ColdJunctionTemperature.getConvertedValue()).
//      //         write(" C, Chip = ").write(chipTemperature.getConvertedValue()).
//      writeln();
//      USBDM::console.resetFormat();
   }
};

#endif /* SOURCES_CHANNEL_H_ */
