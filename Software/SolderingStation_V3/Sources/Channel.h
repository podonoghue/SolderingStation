/*
 * Channel.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CHANNEL_H_
#define SOURCES_CHANNEL_H_

#include "PidController.h"
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

/**
 * Class representing a channel
 */
class Channel {

   friend StepResponseDriver;

private:
   int               toolIdleTime        = 0;        ///< How long the tool has been idle
   float             currentTemperature  = 0;        ///< Measured temperature of tool
   ChannelState      state               = ch_off;   ///< State of channel ch_off/ch_standby/ch_backoff/ch_active
   int               targetTemperature   = 0;        ///< Desired temperature of tool
   unsigned          preset              = 1;        ///< Currently selected preset for the channel

   /// Front panel channel selected LED
   const USBDM::Gpio       &led;

   /// Channel dual drive
   const USBDM::GpioField  &chDrive;

   // Supported irons
   DummyMeasurement  dummyMeasurement{*this};
   Weller_WT50       wellerMeasurement{*this};
   T12               t12Measurement{*this};

public:

   /// Number of preset temperatures provided
   static constexpr unsigned NUM_PRESETS  = 3;

   /// Reference to non-volatile settings stored in Flash
   ChannelSettings  &nvSettings;

   /// Measurement class
   Measurement *measurement;

   friend class Control;

   /**
    * Constructor for channel
    *
    * @param [in] settings    Non-volatile channel settings to associate with this channel
    * @param [in] led         Channel LED
    * @param [in] chDrive     Channel drive
    */
   Channel(ChannelSettings &settings, const USBDM::Gpio &led, const USBDM::GpioField &chDrive) :
      led(led),
      chDrive(chDrive),
      nvSettings(settings),
      measurement(&dummyMeasurement) {
      setUserTemperature(settings.presets[preset]);
      refreshControllerParameters();
      setTip(nvSettings.selectedTip);
      chDrive.write(0b00);
      led.off();
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
            measurement = &dummyMeasurement;
//            usbdm_assert(false, "Illegal iron type");
      }
   }

   /**
    * Inform channel of ID value measurement
    * (Unused)
    *
    * @param value Value from ADC
    */
   void setIdValue(unsigned value) {
      (void) value;
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
      measurement->setCalibrationValues(ts);
   }

   /**
    * Change selected tip for this channel
    *
    * @param delta Offset from current tip in tip settings table
    */
   void changeTip(int delta) {
      setTip(tips.changeTip(nvSettings.selectedTip, delta));
   }

   /**
    * Set selected tip for this channel
    *
    * @param tipSettings TipSettings for tip selection
    */
   void setTip(const TipSettings *tipSettings) {
      usbdm_assert(tipSettings != nullptr, "Illegal tip");
      nvSettings.selectedTip = tipSettings;
      refreshControllerParameters();
      setIronType(tipSettings->getIronType());
      measurement->setCalibrationValues(tipSettings);
   }
   /**
    * Get selected tip for this channel
    *
    * @return Index into tip settings table
    */
   const TipSettings* getTip() const {
      return nvSettings.selectedTip;
   }

   /**
    * Get name of currently selected tip
    *
    * @return Pointer to tip name as static object
    */
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
    * @return Pointer to name of state as static object
    */
   static const char *getStateName(ChannelState state) {
      static const char * const names[] = {
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
    * Get human readable name of current channel state
    *
    * @return Pointer to name of state as static object
    */
   const char *getStateName() const {
      return getStateName(getState());
   }

   /**
    * Indicates the presence of a tip
    *
    * @return
    */
   bool isTipPresent() const {
      return measurement->isTipPresent();
   }

   /**
    * Gets state of channel modified by physical status of tip
    *
    * @return Status value
    */
   ChannelState getState() const {
      if (!isTipPresent()) {
         return ch_noTip;
      }
      return state;
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

      measurement->enableControlLoop(isControlled());
      led.write(isRunning());

      if (state != ch_setback) {
         restartIdleTimer();
      }
      if (!isRunning()) {
         measurement->setDutyCycle(0);

         // For safety while debugging immediately turn off drive
         chDrive.write(0b00);
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
      measurement->enableControlLoop(false);
      measurement->setDutyCycle(0);
      chDrive.write(0b00);
   }

   /**
    * Indicates if the state indicates running i.e. in active, back-off or fixed-power states
    *
    * @param state   State to examine
    *
    * @return  true if active
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
    *
    * @return  true if active
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
    * Sets the user target temperature.
    * Restart timer is cleared
    *
    * @param targetTemp The temperature to set.
    */
   void setUserTemperature(int targetTemp) {
      targetTemperature = targetTemp;
      restartIdleTimer();
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
    * Run end of cycle update:
    *   - Update drive
    *   - Temperature
    *   - Power
    *   - Controller
    */
   void update() {

      // Update drive to heaters as needed
      chDrive.write(measurement->update(targetTemperature));

      // Update current temperature from internal averages
      currentTemperature = measurement->getTemperature();
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

   /**
    * Indicates if the tool has been unused (off) for a while.
    *
    * @return True  - Unused
    * @return False - Used
    */
   bool isToolIdle() {
      return !isRunning() && (toolIdleTime >= 100);
   }

   void report(bool doHeading = false) {
      measurement->report(doHeading);

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

   /**
    * Set output duty cycle.
    * This is only applicable when in fixed power mode
    *
    * @param dutyCycle Duty cycle to set
    */
   void setDutyCycle(unsigned dutyCycle) {
      usbdm_assert(state == ch_fixedPower, "");
      measurement->setDutyCycle(dutyCycle);
   }
};

#endif /* SOURCES_CHANNEL_H_ */
