/*
 * Measurement.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_MEASUREMENT_H_
#define SOURCES_MEASUREMENT_H_

#include "TipSettings.h"
#include "stringFormatter.h"

class Measurement {
protected:
   static constexpr unsigned BUF_SIZE = 40;

   /// Static buffer used for reporting calibration values.
   /// Static as used as return value.
   /// Note - hence usage is not re-entrant!!!
   static USBDM::StringFormatter_T<BUF_SIZE> &getStringFormatter() {
      static USBDM::StringFormatter_T<BUF_SIZE> fStringFormatter;
      return fStringFormatter;
   }

public:
   Measurement() {}

   virtual ~Measurement() {}

   /**
    * Accumulate tip sensor measurement
    *
    * @param value
    */
   virtual void tipSensorAccumulate(int value) = 0;

   /**
    * Accumulate auxiliary sensor measurement
    */
   virtual void auxliliarySensorAccumulate(int) = 0;

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() = 0;

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() = 0;

   /**
    * Indicates if bias current needs to be provided when doing a thermocouple measurement.
    * This is true when using the thermocouple amplifier to measure a low-value thermistor.
    *
    * @return True  => Bias required
    * @return False => Bias not required
    */
   virtual bool isBiasRequired() { return false; }

   /**
    * Get value for resistance of heating element (for power calculations)
    *
    * @return Resistance in ohms
    */
   virtual float getHeaterResistance()  = 0;

   /**
    * Set calibration data from current measurements
    *
    * @param calibrationIndex Index for the calibration
    * @param tipSettings      Tip-settings to update
    */
   virtual void setCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) = 0;

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @return Point to static string
    *
    * @note Not re-entrant!
    */
   virtual const char *reportCalibrationValues() = 0;

   virtual float getMeasurement() = 0;

};

class DummyMeasurement : public Measurement {
public:
   virtual void  tipSensorAccumulate(int) override {};
   virtual void  auxliliarySensorAccumulate(int) override {};
   virtual float getHeaterResistance() override { return 8.0; };
   virtual float getTemperature() override { return 1.0;};
   virtual float getInstantTemperature() override { return 1.0; };
   virtual void  setCalibrationPoint(CalibrationIndex, TipSettings &) override {};
   virtual const char *reportCalibrationValues() override { return ""; };
   virtual float getMeasurement() override { return 0.0; };
};

#endif /* SOURCES_MEASUREMENT_H_ */