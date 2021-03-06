/*
 * WellerWT50.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_WELLER_H_
#define SOURCES_WELLER_H_

#include "stringFormatter.h"

#include "Measurement.h"
#include "Averaging.h"
#include "PidController.h"

/**
 * Class representing information for a Weller WT-50 soldering tweezers
 */
class Weller : public Measurement {

private:

   /// Thermistor in heater
   WellerThermistorAverage thermistor;

   /// Measurement to use for thermistor on sub-channel B
   static constexpr MuxSelect Measurement1_Thermistor =
         muxSelectAddSubChannel(WellerThermistorAverage::MEASUREMENT, SubChannelNum_B);

   /// Loop controller
   PidController controller{CONTROL_INTERVAL, MIN_DUTY, MAX_DUTY};

public:
   Weller(Channel &ch) : Measurement(ch, 11.0, 24) {}

   virtual ~Weller() {}

   /**
    * Enable control loop
    *
    * @param[in] enable True to enable
    */
   virtual void enableControlLoop(bool enable) override {
      controller.enable(enable);
   }

   /**
    * Run end of controller cycle update:
    *   - Temperature
    *   - Power
    *   - Controller (PID etc)
    *
    * @param targetTemperature Target temperature
    */
   virtual void updateController(float targetTemperature) override {

      // Run PID
      float dc = controller.newSample(targetTemperature, getTemperature());
      controller.setDutyCycle(dc);
   }

   /**
    * Get drive value for each main half-cycle
    *
    * @return Drive value for channel
    */
   virtual DriveSelection getDrive() override {

      // Update drive to heaters as needed
      controller.advance();

      // Update power average (as percentage)
      power.accumulate(controller.getDutyCycle());

      // Get output value
      return controller.isOn()?DriveSelection_Both:DriveSelection_Off;
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() const override {
      return thermistor.getTemperature();
   }

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() const override {
      return thermistor.getInstantTemperature();
   }

   /**
    * Set calibration data from current measurements
    *
    * @param calibrationIndex Index for the calibration
    * @param tipSettings      Tip-settings to update
    *
    * @return true  - Success
    * @return false - Fail - values appear out of range
    */
   virtual bool saveCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) override {
      float resistance = thermistor.getResistance();
      if ((resistance < 20) || (resistance > 80)) {
         return false;
      }
      tipSettings.setCalibrationPoint(
            calibrationIndex,
            TipSettings::getCalibrationTemperature(calibrationIndex),
            thermistor.getResistance());
      controller.setControlParameters(&tipSettings);
      return true;
   }

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @param io      Where to write report to
    * @param brief   True to shorten report (2 lines maximum)
    */
   virtual void reportCalibrationValues(USBDM::FormattedIO &io, bool brief) const override {
      (void) brief;
      io.setFloatFormat(1);
      io.write("R = ").write(thermistor.getResistance()).writeln(" ohms");
      io.write("T = ").write(thermistor.getTemperature()).writeln(" C");
   }

   /**
    * Get default calibration and control values
    *
    * @param settings
    * @param initInfo
    */
   static void initialiseSettings(TipSettings *settings, const InitialTipInfo &initInfo) {

      // Dummy values                                       Tt(C)    R(ohms)
      settings->setCalibrationPoint(CalibrationIndex_250,   250,     37.78); // 38.39
      settings->setCalibrationPoint(CalibrationIndex_325,   325,     40.83); // 41.43
      settings->setCalibrationPoint(CalibrationIndex_400,   400,     43.37); // 43.55

      if (strcmp(initInfo.name, "WT50S") == 0) {
         // Small tips - Kp=1.0,Ki=0.03,Kd=0.0,Ilimit=20@100%
         settings->setInitialPidControlValues(1.0,0.03,0.0,20.0);
      }
      else if (strcmp(initInfo.name, "WT50M") == 0) {
         // Medium tips - Kp=1.5,Ki=0.03,Kd=0.0,Ilimit=35@100%
         settings->setInitialPidControlValues(1.5,0.03,0.0,35.0);
      }
      else if (strcmp(initInfo.name, "WT50L") == 0) {
         // Large tips - Kp=2.0,Ki=0.06,Kd=0.0,Ilimit=32.5@100%
         settings->setInitialPidControlValues(2.0,0.06,0.0,32.5);
      }
      else if (strcmp(initInfo.name, "WSP80") == 0) {
         // Standard iron - Kp=2.0,Ki=0.06,Kd=0.0,Ilimit=32.5@100%
         settings->setInitialPidControlValues(0.5,0.05,0.0,20.0);
      }
   }

   /**
    * Set calibration data
    *
    * @param[in] tipsettings Settings object with calibration data
    */
   virtual void setCalibrationValues(const TipSettings *tipsettings) override {
      thermistor.setCalibrationValues(tipsettings);
      controller.setControlParameters(tipsettings);
   }

   /**
    * Get the sequence of ADC measurements to do
    *
    * @return Sequence array terminated by MuxSelect_Complete sentinel
    */
   virtual MuxSelect const *getMeasurementSequence() const override {
      static const MuxSelect sequence[] = {
            Measurement1_Thermistor,
            MuxSelect_Complete,
      };
      return sequence;
   };

   /**
    * Process ADC value
    *
    * @param[in] muxSelect  Indicates measurement made
    * @param[in] adcValue   ADC value from measurement
    */
   virtual void processMeasurement(MuxSelect muxSelect, uint32_t adcValue) override {
      (void) adcValue;
      switch(muxSelect) {
         case Measurement1_Thermistor:
            // Thermistor
            tipPresent = thermistor.accumulate(adcValue);
            break;

         default:
            // Can occur briefly when changing tools
            break;
      }
   };

   /**
    * Set duty cycle for use with fixed power mode
    *
    * @param dutyCycle
    */
   virtual void setDutyCycle(unsigned dutyCycle) {
      controller.setOutput(dutyCycle);
   }

   /**
    * Print single line report on control situation
    *
    * @param doHeading True to print a header first
    */
   virtual void report(bool doHeading=false) const {
      using namespace USBDM;

      if (doHeading) {
         console.write("Time,");
         controller.reportHeading(ch);
      }

      // Time-stamp
      console.setFloatFormat(2, Padding_LeadingSpaces, 3);
      console.write(controller.getElapsedTime());
      controller.report();

      console.writeln(",", getInstantTemperature());
      console.resetFormat();
   }

};

#endif /* SOURCES_WELLER_H_ */
