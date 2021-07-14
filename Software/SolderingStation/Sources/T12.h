/*
 * T12.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef T12_H_
#define T12_H_

#include "Measurement.h"
#include "Averaging.h"

class T12 : public Measurement {

private:
   /// Thermistor in handle
   ThermistorMF58Average coldJunctionThermistor;

   /// Thermocouple in cartridge
   ThermocoupleAverage   thermocouple;

public:
   T12() {}
   virtual ~T12() {}

   /**
    * Accumulate tip sensor measurement
    *
    * @param value
    */
   virtual void tipSensorAccumulate(int value) override {
      thermocouple.accumulate(value);
   }

   /**
    * Accumulate auxiliary sensor measurement
    */
   virtual void auxliliarySensorAccumulate(int value) override {
      coldJunctionThermistor.accumulate(value);
   }

   /**
    * Get value for resistance of heating element (for power calculations)
    *
    * @return Resistance in ohms
    */
   virtual float getHeaterResistance() override {
      return 8.5;
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() override {
      return thermocouple.getTemperature()+coldJunctionThermistor.getTemperature();
   }

   /**
    * Set set default calibration values
    *
    * @param tipNameIndex  Tip name index for this setting
    */
   virtual void initialiseSettings(TipSettings *settings, TipSettings::TipNameIndex) {

      // Dummy values
      settings->setTemperatureValues(TipSettings::Calib_250, 25, 5.4);
      settings->setTemperatureValues(TipSettings::Calib_250, 25, 5.4);
      settings->setTemperatureValues(TipSettings::Calib_250, 25, 5.4);

      settings->setPidControlValues(5.0,0.5,0.0,40.0);
   }
};

#endif /* T12_H_ */
