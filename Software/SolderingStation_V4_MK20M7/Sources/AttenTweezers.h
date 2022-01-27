/*
 * JBC.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef ATTEN_TWEEZERS_H
#define ATTEN_TWEEZERS_H

#include "stringFormatter.h"

#include "Measurement.h"
#include "Averaging.h"
#include "PidController.h"
#include "TakeBackHalfController.h"

/**
 * Class representing information for a JBC soldering iron
 */
class AttenTweezers : public Measurement {

private:

   using ThermocoupleAveraging = ThermocoupleAverage<20>; // 20 * 20 ms = 400 ms

   /// Measurement to use for thermocouple on sub-channel A
   static constexpr MuxSelect Measurement1_LeftThermocouple = muxSelectAddSubChannel(ThermocoupleAveraging::MEASUREMENT, SubChannelNum_A);

   /// Measurement to use for thermocouple on sub-channel B
   static constexpr MuxSelect Measurement2_RightThermocouple = muxSelectAddSubChannel(ThermocoupleAveraging::MEASUREMENT, SubChannelNum_B);

   /// Thermocouple in cartridge
   ThermocoupleAveraging   leftThermocouple;
   ThermocoupleAveraging   rightThermocouple;

   /// Loop controller
   PidController leftController{2*SAMPLE_INTERVAL, MIN_DUTY, MAX_DUTY};
   PidController rightController{2*SAMPLE_INTERVAL, MIN_DUTY, MAX_DUTY};

//   TakeBackHalfController leftController{2*SAMPLE_INTERVAL, MIN_DUTY, MAX_DUTY};
//   TakeBackHalfController rightController{2*SAMPLE_INTERVAL, MIN_DUTY, MAX_DUTY};

public:
   AttenTweezers(Channel &ch) : Measurement(ch, 3.3/2.0, 12) {}

   virtual ~AttenTweezers() {}

   /**
    * Enable control loop
    *
    * @param[in] enable True to enable
    */
   virtual void enableControlLoop(bool enable) override {
      leftController.enable(enable);
      rightController.enable(enable);
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

      // Run PIDs
      float leftDc  = leftController.newSample(targetTemperature, getLeftTemperature());
      float rightDc = rightController.newSample(targetTemperature, getRightTemperature());

      leftController.setDutyCycle(leftDc);
      rightController.setDutyCycle(rightDc);
   }

   /**
    * Get drive value for each mains half-cycle
    *
    * @return Drive value for channel
    */
   virtual DriveSelection getDrive() override {

      // Update drive to heaters as needed
      leftController.advance();
      rightController.advance();

      // Update power average (as percentage)
      power.accumulate((leftController.getDutyCycle()+rightController.getDutyCycle())/2);

      // Get output value
      return (leftController.isOn()?DriveSelection_Left:DriveSelection_Off)|
             (rightController.isOn()?DriveSelection_Right:DriveSelection_Off);
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   float getLeftTemperature() const {
      return leftThermocouple.getTemperature();
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   float getRightTemperature() const {
      return rightThermocouple.getTemperature();
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() const override {
      return (leftThermocouple.getTemperature()+rightThermocouple.getTemperature())/2;
   }

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() const override {
      return (leftThermocouple.getInstantTemperature()+rightThermocouple.getInstantTemperature())/2;
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
      using namespace USBDM;

      float thermocoupleVoltage_mV  = 1000*leftThermocouple.getThermocoupleVoltage();

      console.writeln(calibrationIndex, " : TC = ", thermocoupleVoltage_mV);

      if ((thermocoupleVoltage_mV < 0.5) || (thermocoupleVoltage_mV > 3)) {
         return false;
      }

      float thermocoupleTemperature = TipSettings::getCalibrationTemperature(calibrationIndex);
      tipSettings.setCalibrationPoint(calibrationIndex, thermocoupleTemperature, thermocoupleVoltage_mV);

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
      float lThermocoupleVoltage  = leftThermocouple.getThermocoupleVoltage();
      float lThermocoupleTemp     = leftThermocouple.getTemperature();
      float rThermocoupleVoltage  = rightThermocouple.getThermocoupleVoltage();
      float rthermocoupleTemp     = rightThermocouple.getTemperature();

      io.
         setFloatFormat(1, USBDM::Padding_LeadingSpaces, 3).
         write("TC=", lThermocoupleTemp, " C (").
         setFloatFormat(1).
         writeln(1000*lThermocoupleVoltage, " mV)");
      io.
         setFloatFormat(1, USBDM::Padding_LeadingSpaces, 3).
         write("TC=", rthermocoupleTemp, " C (").
         setFloatFormat(1).
         writeln(1000*rThermocoupleVoltage, " mV)");
      if (!brief) {
         io.
            setFloatFormat(1, USBDM::Padding_LeadingSpaces, 3).
            write("T =").write(getTemperature()).write(" C");
      }
   }

   /**
    * Get default calibration and control values
    *
    * @param settings
    * @param initInfo
    */
   static void initialiseSettings(TipSettings *settings, const InitialTipInfo &initInfo) {

      (void)initInfo;
      // Dummy values                                      Tt(C)   Vt(mV)
      settings->setCalibrationPoint(CalibrationIndex_250,  250.0,  4.3);
      settings->setCalibrationPoint(CalibrationIndex_325,  325.0,  5.6);
      settings->setCalibrationPoint(CalibrationIndex_400,  400.0,  6.85);

      settings->setInitialPidControlValues(1.1,0.1,1.0,20.0);
   }

   /**
    * Set calibration data
    *
    * @param[in] tipsettings Settings object with calibration data
    */
   virtual void setCalibrationValues(const TipSettings *tipsettings) override {
      leftThermocouple.setCalibrationValues(tipsettings);
      leftController.setControlParameters(tipsettings);
      rightThermocouple.setCalibrationValues(tipsettings);
      rightController.setControlParameters(tipsettings);
   }

   /**
    * Get the sequence of ADC measurements to do
    *
    * @return Sequence array terminated by MuxSelect_Complete sentinel
    */
   virtual MuxSelect const *getMeasurementSequence() const override {
      static const MuxSelect sequence[] = {
            Measurement1_LeftThermocouple,
            Measurement2_RightThermocouple,
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

         case Measurement1_LeftThermocouple:
            // Thermocouple
            tipPresent = leftThermocouple.accumulate(adcValue);
            break;


         case Measurement2_RightThermocouple:
            // Thermocouple
            tipPresent = rightThermocouple.accumulate(adcValue);
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
      leftController.setOutput(dutyCycle);
      rightController.setOutput(dutyCycle);
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
         leftController.reportHeading(ch);
      }

      // Time-stamp
      console.setFloatFormat(2, Padding_LeadingSpaces, 3);
      console.write(leftController.getElapsedTime());
      leftController.report();

      console.writeln(",", getInstantTemperature());
      console.resetFormat();
   }

};

#endif /* ATTEN_TWEEZERS_H */
