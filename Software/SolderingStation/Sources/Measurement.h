/*
 * Measurement.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_MEASUREMENT_H_
#define SOURCES_MEASUREMENT_H_

#include "TipSettings.h"

class Measurement {

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
    * Set set default calibration values
    *
    * @param tipNameIndex  Tip name index for this setting
    */
   virtual void initialiseSettings(TipSettings *settings, TipSettings::TipNameIndex tipNameIndex) = 0;
};



#endif /* SOURCES_MEASUREMENT_H_ */
