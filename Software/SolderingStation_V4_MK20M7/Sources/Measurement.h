/*
 * Measurement.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_MEASUREMENT_H_
#define SOURCES_MEASUREMENT_H_

#include "TipSettings.h"
#include "Averaging.h"

class Channel;

class Measurement {

public:

   /// Resistance of heater element in ohms
   const float heaterResistance;

   /// Voltage of heater element in volts
   const unsigned heaterVoltage;

   /// Maximum power calculated from heater resistance and voltage
   const float nominalMaxPower;

protected:

   /// Owning channel
   Channel &ch;

   /// Indicates if the tip is present
   bool tipPresent;

   /// Average power as percentage for display purposes
   SimpleMovingAverage<5> power;

public:
   Measurement(Channel &ch, float heaterResistance, unsigned heaterVoltage) :
      heaterResistance(heaterResistance),
      heaterVoltage(heaterVoltage),
      nominalMaxPower((heaterVoltage * heaterVoltage)/heaterResistance),
      ch(ch),
      tipPresent(false) {

   }

   virtual ~Measurement() {}

   /**
    * Indicates if the tip is present
    *
    * @return
    */
   bool isTipPresent() const {
      return tipPresent;
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() const = 0;

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() const = 0;

   /**
    * Set calibration data from current measurements
    *
    * @param[in]  calibrationIndex Index for the calibration
    * @param[out] tipSettings      Tip-settings to update with calibratin point
    */
   virtual bool saveCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) = 0;

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @param io      Where to write report to
    * @param brief   True to shorten report (2 lines maximum)
    */
   virtual void reportCalibrationValues(USBDM::FormattedIO &io, bool brief=false) const = 0;

   /**
    * Set calibration data
    *
    * @param[in] tipsettings Settings object with calibration data
    */
   virtual void setCalibrationValues(const TipSettings *tipsettings) = 0;

   /**
    * Get the sequence of ADC measurements to do
    *
    * @return Sequence array terminated by MuxSelect_Complete sentinel
    */
   virtual MuxSelect const *getMeasurementSequence() const = 0;

   /**
    * Process ADC measurement value
    *
    * @param[in] muxSelect  Indicates which measurement made.\n
    *            Assumes channel information has been stripped.
    * @param[in] adcValue   ADC value for measurement
    */
   virtual void processMeasurement(MuxSelect muxSelect, uint32_t adcValue) = 0;

   /**
    * Enable control loop
    *
    * @param[in] enable True to enable
    */
   virtual void enableControlLoop(bool enable = true) = 0;

   /**
    * Run end of controller cycle update:
    *   - Temperature
    *   - Power
    *   - Controller (PID etc)
    *
    * @param targetTemperature Target temperature
    */
   virtual void updateController(float targetTemperature) = 0;

   /**
    * Get drive value for each main half-cycle
    *
    * @return Drive value for channel
    */
   virtual DriveSelection getDrive() = 0;

   /**
    * Get average power
    *
    * @return Power in watts
    */
   float getPower() const {
      return power.getAveragedAdcSamples()*nominalMaxPower/100;
   }

   /**
    * Get average power
    *
    * @return Power as percentage
    */
   float getPercentagePower() const {
      return power.getAveragedAdcSamples();
   }

   /**
    * Set duty cycle for use with fixed power mode
    *
    * @param dutyCycle
    */
   virtual void setDutyCycle(unsigned dutyCycle) = 0;

   /**
    * Print single line report on control situation
    *
    * @param doHeading True to print a header first
    */
   virtual void report(bool doHeading=false) const = 0;
};

class DummyMeasurement : public Measurement {
public:
   DummyMeasurement(Channel &ch) : Measurement(ch, 8.0, 0) {}

   virtual float getTemperature() const override { return 1.0;};
   virtual float getInstantTemperature() const override { return 1.0; };
   virtual bool  saveCalibrationPoint(CalibrationIndex, TipSettings &) override { return false; };
   virtual void  reportCalibrationValues(USBDM::FormattedIO &, bool) const override {};
   virtual void setCalibrationValues(const TipSettings *) override { }
   virtual MuxSelect const *getMeasurementSequence() const override {
      static const MuxSelect dummy[] = {MuxSelect_Complete, };
      return dummy;
   }
   virtual void processMeasurement(MuxSelect, uint32_t) override {}
   virtual void enableControlLoop(bool) override {}
   virtual void updateController(float) override {}
   virtual DriveSelection getDrive() override { return DriveSelection_Off; }
   virtual void setDutyCycle(unsigned) {}
   virtual void report(bool) const {}
};

#endif /* SOURCES_MEASUREMENT_H_ */
