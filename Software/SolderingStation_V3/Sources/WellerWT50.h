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
    * Get default calibration and control values
    *
    * @param settings
    * @param initInfo
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

   /**
    * Get the sequence of ADC measurements to do
    *
    * @return Sequence array terminated by MuxSelect_Complete sentinel
    */
   virtual MuxSelect const *getMeasurementSequence() override {
      static const MuxSelect sequence[] = {
            MuxSelect_ChaLowGainBiased,
            MuxSelect_Complete,
      };
      return sequence;
   };

   /**
    * Process ADC value
    *
    * @param[in] muxSelect  Indicates measurement made
    * @param[in] adcValue   ADC value for measurement
    */
   virtual void processMeasurement(MuxSelect muxSelect, uint32_t adcValue) override {
      (void) adcValue;
      switch(muxSelect) {
         case MuxSelect_ChaLowGainBiased:
            // Thermistor
            thermistor.accumulate(adcValue);
            break;

         default:
            usbdm_assert(false, "Unexpected measurement");
            break;
      }
   };

};

#endif /* SOURCES_WELLERWT50_H_ */
