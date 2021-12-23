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
   ch_noTool,     ///< No tool detected
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
   // How long the tool has been idle
   int               toolIdleTime        = 0;

   // Measured temperature of tool
   float             currentTemperature  = 0;

   // State of channel ch_off/ch_standby/ch_backoff/ch_active
   ChannelState      state               = ch_off;

   // Desired temperature of tool
   int               targetTemperature   = 0;

   // Currently selected preset for the channel
   unsigned          preset              = 1;

   // Current iron type  - Updated from detectedIronType in polling thread
   IronType          ironType         = IronType_Unknown;

   // Counter to keep track of when to do tool identification
   unsigned identifyCounter = 0;

   // Front panel channel selected LED
   const USBDM::Gpio       &led;

   // Channel dual drive
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
   Measurement *measurement = &dummyMeasurement;

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
      nvSettings(settings) {
      setUserTemperature(settings.presets[preset]);
      chDrive.write(0b00);
      led.off();
      checkTipSelected();
   }

   virtual ~Channel() {}

   /**
    * Set type of soldering iron being used
    *
    *    If the type is Unknown then the channel state is changed to NoTool.
    *    If the type has changed then the channel state is changed to off
    *
    * @param ironType  Iron type to set
    */
   void setIronType(IronType ironType) {

      if (ironType != this->ironType) {

         // Tool type changed
//         USBDM::console<<"Tool changed to "<<TipSettings::getIronTypeName(ironType)<<'\n';
         if (ironType == IronType_Unknown) {
            setState(ch_noTool);
         }
         else {
            // Turn off on tool type change
            setState(ch_off);
         }
      }
      // Update iron type and measurement handler
      this->ironType = ironType;
      switch(ironType) {
         case IronType_T12:
            measurement = &t12Measurement;
            break;
         case IronType_Weller:
            measurement = &wellerMeasurement;
            break;
         default:
            measurement = &dummyMeasurement;
      }
      checkTipSelected();
   }

   /**
    * Get the sequence of ADC measurements to do
    *
    * @param[out] seq         Array of measurements to do
    * @param[in]  channelMask Mask indicating which channel
    *
    * @return Number of measurements added to seq[]
    */
   unsigned getMeasurementSequence(MuxSelect seq[], uint8_t channelMask) {
      static const MuxSelect identifySequence[] = {
            MuxSelect_Identify,
            MuxSelect_Complete,
      };
      MuxSelect const *newSequence;
      if (!isRunning() && (identifyCounter++ > 20)) {
         // Regularly check for tool change
         newSequence = identifySequence;
         identifyCounter = 0;
      }
      else if (getState() == ch_noTool) {
         // No tool present - no measurements
         return 0;
      }
      else {
         // Get sequence dependent on tool type
         newSequence = measurement->getMeasurementSequence();
      }
      unsigned sequenceLength;
      for(sequenceLength=0; newSequence[sequenceLength] != MuxSelect_Complete; sequenceLength++) {
         // Add channel information
         seq[sequenceLength] = (MuxSelect)(newSequence[sequenceLength]|channelMask);
      }
      return sequenceLength;
   }

   /**
    * Find nearest E12 resistor in range 1k - 10k.
    *
    * @param value
    *
    * @return Rounded value or zero if not found/invalid
    */
   int getE12Value(int value) {
      static const int e12Values[] = {
        0, 1000,1200,1500,1800,2200,2700,3300,3900,4700,5600,6800,8200,10000,
      };
      static const int squareOfGeometricMean[] = {
        (int)round(8.2*10), 10*12,12*15,15*18,18*22,22*27,27*33,33*39,39*47,47*56,56*68,68*82,82*100,100*120
      };
      // Keep in range (with rounding)
      value = (value+99)/100;

      // Compare squared value with squared geometric mean
      value *= value;

      for(unsigned index=0; index<USBDM::sizeofArray(e12Values); index++) {
         if (value<squareOfGeometricMean[index]) {
            return e12Values[index];
         }
      }
      return 0;
   }

   /**
    * Process ADC measurement value
    *
    * @param[in] muxSelect  Indicates which measurement made.
    * @param[in] adcValue   ADC value for measurement
    */
   void processMeasurement(MuxSelect muxSelect, uint32_t adcValue) {

      // Strip channel information
      muxSelect = static_cast<MuxSelect>(muxSelect&~CHANNEL_MASK);

      if (muxSelect == MuxSelect_Identify) {

         // Tool type check

         constexpr float Vref = 3.3; // volts
         constexpr float Rs   = 22000;
         float Vt = (adcValue*ADC_REF_VOLTAGE)/USBDM::FixedGainAdc::getSingleEndedMaximum(ADC_RESOLUTION)/2; // volts

         // Calculate ID resistor from voltage divider
         int Rt = round(Vt*Rs/(Vref-Vt));

         // Round to nearest E12 value
         int roundedRt = getE12Value(Rt);

         IronType  detectedIronType = IronType_Unknown;

         switch(roundedRt) {
            case  2200 : detectedIronType = IronType_T12;      break;
            case 10000 : detectedIronType = IronType_Weller;   break;
            default    : detectedIronType = IronType_Unknown;  break;
         }
         setIronType(detectedIronType);
//         USBDM::console.write("Identify: ").write("R = ").writeln(Rt);
//         USBDM::console.write("Identify: ").write("R = ").write(Rt).write(", ").write(roundedRt).write(" => ").writeln(TipSettings::getIronTypeName(detectedIronType));
      }
      else {
         // Pass to tool specific handling
         measurement->processMeasurement(muxSelect, adcValue);
      }
   }

   /**
    * Check for a valid tip selection and re-assign if necessary
    */
   void checkTipSelected() {
      if (ironType == IronType_Unknown) {
         // Any tip is valid until an iron is present
         return;
      }
      // Update tip selection if needed
      const TipSettings *ts = nvSettings.selectedTip;
      if ((ts == nullptr)|| ts->isFree() || (ts->getIronType() != ironType)) {
         setTip(tips.getAvailableTipForIron(ironType));
      }
      refreshControllerParameters();
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
      if (ironType == IronType_Unknown) {
         return;
      }
      setTip(tips.changeTip(nvSettings.selectedTip, delta));
   }

   /**
    * Set selected tip for this channel
    *
    * @param tipSettings TipSettings for tip selection
    */
   void setTip(const TipSettings *tipSettings) {
      if (tipSettings->getIronType() == IronType_Unknown) {
         return;
      }
      usbdm_assert(tipSettings != nullptr, "Illegal tip");
      usbdm_assert(tipSettings->getIronType() == ironType, "Tip not suitable for iron");
      nvSettings.selectedTip = tipSettings;
      refreshControllerParameters();
   }
   /**
    * Get selected tip for this channel
    *
    * @return Index into tip settings table
    */
   const TipSettings* getTip() const {
      if (ironType == IronType_Unknown) {
         return &Tips::NoTipSettings;
      }
      return nvSettings.selectedTip;
   }

   /**
    * Get name of currently selected tip
    *
    * @return Pointer to tip name as static object
    */
   const char *getTipName() {
      if (ironType == IronType_Unknown) {
         return Tips::NoTipSettings.getTipName();
      }
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
            "No Tool",
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
      if (ironType == IronType_Unknown) {
         return ch_noTool;
      }
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

      state = newState;

      measurement->enableControlLoop(isControlled());
      led.write(isRunning());

      refreshControllerParameters();

      measurement->enableControlLoop(isControlled());
      led.write(isRunning());

      if (newState != ch_setback) {
         restartIdleTimer();
      }
      if (!isRunning()) {
         measurement->setDutyCycle(0);

         // For safety while debugging immediately turn off drive
         chDrive.write(0b00);
      }
      else {
         const TipSettings *ts = nvSettings.selectedTip;
         (void) ts;
         usbdm_assert((ts != nullptr) && !ts->isFree() && (ts->getIronType() == ironType), "Wrong tip selected");
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
    * Indicates if the set temperature is different to the last preset selected.
    *
    * @return True if temperature differs
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

      if ((getState() == ch_active) && (nvSettings.setbackTime > 0) && (toolIdleTime >= nvSettings.setbackTime*1000)) {
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
      usbdm_assert(getState() == ch_fixedPower, "Only available in Fixed power state");
      measurement->setDutyCycle(dutyCycle);
   }

   /**
    * Get type of iron attached to channel
    *
    * @return Type of connected iron
    */
   IronType getIronType() {
      return ironType;
   }
};

#endif /* SOURCES_CHANNEL_H_ */
