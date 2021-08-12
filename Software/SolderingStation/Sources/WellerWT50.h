/*
 * WellerWT50.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_WELLERWT50_H_
#define SOURCES_WELLERWT50_H_

#include "Measurement.h"
#include "Averaging.h"
#include "stringFormatter.h"

/**
 * Class representing information for a Weller WT-50 soldering tweezers
 */
class Weller_WT50 : public Measurement {

private:

   WellerThermistorAverage thermistor;

public:
   Weller_WT50() {}

   virtual ~Weller_WT50() {}

   /**
    * Accumulate tip sensor measurement
    *
    * @param value
    */
   virtual void tipSensorAccumulate(int value) override {
      thermistor.accumulate(value);
   }

   /**
    * Accumulate auxiliary sensor measurement
    */
   virtual void auxliliarySensorAccumulate(int) override {
      // Not used
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() override {
      return thermistor.getTemperature();
   }

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() override {
      return thermistor.getInstantTemperature();
   }

   /**
    * Indicates if bias current needs to be provided when doing a thermocouple measurement.
    * This is true when using the thermocouple amplifier to measure a low-value thermistor.
    *
    * @return True  => Bias required
    * @return False => Bias not required
    */
   virtual bool isBiasRequired() const override {
      return true;
   }

   /**
    * Get value for resistance of heating element (for power calculations)
    *
    * @return Resistance in ohms
    */
   virtual float getHeaterResistance() const override {
      return 11.0;
   }

   /**
    * Set calibration data from current measurements
    *
    * @param calibrationIndex Index for the calibration
    * @param tipSettings      Tip-settings to update
    */
   virtual void saveCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) override {
      tipSettings.setCalibrationPoint(calibrationIndex, thermistor.getResistance(), 0.0);
   }

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @return Point to static string
    *
    * @note Not re-entrant!
    */
   virtual const char *reportCalibrationValues() {
      getStringFormatter().clear();
      getStringFormatter().setFloatFormat(1);
      getStringFormatter().write("R = ").write(thermistor.getResistance()).writeln(" ohms");
      return getStringFormatter().toString();
   }

   /**
    * Set set default calibration values
    *
    * @param tipNameIndex  Tip name index for this setting
    */
   static void initialiseSettings(TipSettings *settings, const InitialTipInfo &initInfo) {

      // Dummy values
      settings->setCalibrationPoint(CalibrationIndex_250, 25, 5.4);
      settings->setCalibrationPoint(CalibrationIndex_325, 25, 5.4);
      settings->setCalibrationPoint(CalibrationIndex_400, 25, 5.4);

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
   }

   /**
    * Set calibration data
    *
    * @param[in] tipsettings Settings object with calibration data
    */
   virtual void setCalibrationValues(const TipSettings *tipsettings) override {
      thermistor.setCalibrationValues(tipsettings);
   }

   /**
    * Get 'measurement' value for debug when calibrating
    *
    * @return
    */
   virtual float getMeasurement() const override {
      return thermistor.getResistance();
   };

};

#endif /* SOURCES_WELLERWT50_H_ */
