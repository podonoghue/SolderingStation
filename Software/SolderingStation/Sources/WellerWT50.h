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

/**
 * Class representing information for a Weller WT-50 soldering tweezers
 */
class Weller_WT50 : public Measurement {

private:

   WellerThermistorAverage thermister;

public:
   Weller_WT50() {}

   virtual ~Weller_WT50() {}

   /**
    * Accumulate tip sensor measurement
    *
    * @param value
    */
   virtual void tipSensorAccumulate(int value) override {
      thermister.accumulate(value);
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
      return thermister.getTemperature();
   }

   /**
    * Indicates if bias current needs to be provided when doing a thermocouple measurement.
    * This is true when using the thermocouple amplifier to measure a low-value thermistor.
    *
    * @return True  => Bias required
    * @return False => Bias not required
    */
   virtual bool isBiasRequired() override {
      return true;
   }

   /**
    * Get value for resistance of heating element (for power calculations)
    *
    * @return Resistance in ohms
    */
   virtual float getHeaterResistance() override {
      return 11.0;
   }

   /**
    * Set set default calibration values
    *
    * @param tipNameIndex  Tip name index for this setting
    */
   static void initialiseSettings(TipSettings *settings, TipSettings::TipNameIndex) {

      // Dummy values
      settings->setTemperatureValues(TipSettings::Calib_250, 25, 5.4);
      settings->setTemperatureValues(TipSettings::Calib_250, 25, 5.4);
      settings->setTemperatureValues(TipSettings::Calib_250, 25, 5.4);

      // Small tips - Kp=1.0,Ki=0.03,Kd=0.0,Ilimit=40
      settings->setPidControlValues(1.0,0.03,0.0,40.0);
   }
};

#endif /* SOURCES_WELLERWT50_H_ */
