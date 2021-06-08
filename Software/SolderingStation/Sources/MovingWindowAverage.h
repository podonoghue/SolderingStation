/*
 * MovingWIndowAverage.h
 *
 *  Created on: 4 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_MOVINGWINDOWAVERAGE_H_
#define SOURCES_MOVINGWINDOWAVERAGE_H_

#include "adc.h"
#include "Peripherals.h"

/**
 * Class representing a moving average window for ADC values.
 *
 * @tparam WindowSize Number of samples to average over
 */
template<unsigned WindowSize>
class MovingWindowAverage {

private:
   /// Samples over entire window (circular buffer)
   unsigned samples[WindowSize] = {0};

   /// Index into samples
   unsigned index = 0;

   /// Summation of samples
   unsigned sum = 0;

   MovingWindowAverage(const MovingWindowAverage &other) = delete;
   MovingWindowAverage(MovingWindowAverage &&other) = delete;
   MovingWindowAverage& operator=(const MovingWindowAverage &other) = delete;
   MovingWindowAverage& operator=(MovingWindowAverage &&other) = delete;

public:
   /**
    * Constructor
    */
   MovingWindowAverage() {}

   /**
    * Destructor
    */
   virtual ~MovingWindowAverage() {}

   /**
    * Add ADC value to window
    *
    * @param value to add - replaces oldest value
    */
   void accumulate(unsigned value) {

      sum += value;
      sum -= samples[index];
      samples[index++] = value;
      if (index >= WindowSize) {
         index = 0;
      }
   }

   /**
    * Return ADC voltage averaged over window
    *
    * @return value calculated over window
    */
   float getAverage() {

      float voltage;

      // Calculate average over window
      voltage = (sum/WindowSize);

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/USBDM::Adc0::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

/**
 * Class representing a moving average window customised for a thermistor
 *
 * @tparam WindowSize Number of samples to average over
 */
template <unsigned WindowSize>
class ThermistorAverage : public MovingWindowAverage<WindowSize> {

private:

   /**
    * Converts thermistor resistance value to temperature
    *
    * @param resistance Thermistor resistance in ohms
    *
    * @return Corresponding temperature in Celsius
    */
   float resistanceToCelsius(double resistance){

      // Value from curve fitting see spreadsheet
      constexpr double A_constant = 1.62770581419817E-03;
      constexpr double B_constant = 6.39056547750702E-05;
      constexpr double C_constant = 1.91419439391882E-05;
      constexpr double D_constant = -6.36504328625315E-07;

      constexpr double KelvinToCelsius = -274.15;

      // Used to calculate R^N
      double log_R_Nth = log(resistance);

      double power;

      // Used to calculate 1/T in Kelvin
      double reciprocalTemperature;

      reciprocalTemperature  = A_constant;
      power  = log_R_Nth;
      reciprocalTemperature += B_constant * power;
      power *= log_R_Nth;
      reciprocalTemperature += C_constant * power;
      power *= log_R_Nth;
      reciprocalTemperature += D_constant * power;

      double temperatureInKelvin = 1/reciprocalTemperature;

      return temperatureInKelvin + KelvinToCelsius;
   }

public:
   using MovingWindowAverage<WindowSize>::getAverage;

   /**
    * Returns the moving window average of the thermistor resistance
    *
    * @return Thermistor resistance in ohms
    */
   float getResistance() {

      // Get ADC value as voltage
      float voltage = getAverage();

      return NTC_MEASUREMENT_RATIO * voltage;
   }

   /**
    * Returns the moving window average of the thermistor temperature in Celsius
    *
    * @return  Thermistor temperature in Celsius
    */
   float getTemperature() {

      return resistanceToCelsius(getResistance());
   }
};

/**
 * Class representing a moving average window customised for a thermocouple
 *
 * @tparam WindowSize Number of samples to average over
 */
template <unsigned WindowSize>
class ThermocoupleAverage : public MovingWindowAverage<WindowSize> {

private:

   using MovingWindowAverage<WindowSize>::getAverage;

   /**
    * Converts a thermocouple voltage to temperature
    *
    * @param voltage Thermocouple voltage
    *
    * @return Temperature in Celsius
    */
   float voltageToCelsius(float voltage) {

      // Curve fitting was done using mV
      voltage *= 1000;

      // Value from curve fitting see spreadsheet
      constexpr double A_constant = -20.3057343295616; // -12.9509284993216;
      constexpr double B_constant =  46.191866056261;  //45.4204980163692;

      // Used to calculate V^N
      double power;
      float temperature;

      temperature = A_constant;
      power = voltage;
      temperature += B_constant*power;

      return temperature;
   }

public:
   /**
    * Returns the moving window average of the thermocouple voltage
    *
    * @return Thermocouple voltage in V
    */
   float getVoltage() {

      // Get ADC value as voltage
      float voltage = getAverage();

      return voltage * TC_MEASUREMENT_RATIO;
   }

   /**
    * Returns the moving window average of the thermocouple temperature
    *
    * @return Thermocouple temperature in Celsius
    */
   virtual float getTemperature() {

      return voltageToCelsius(getVoltage());
   }
};

/**
 * Class representing a moving average window customised for the internal temperature sensor
 *
 * @tparam WindowSize Number of samples to average over
 */
template <unsigned WindowSize>
class ChipTemperatureAverage : public MovingWindowAverage<WindowSize> {

private:

   /**
    * Convert ADC voltage measurement to Internal chip temperature
    *
    * @param voltage measurement
    *
    * @return Chip temperature in Celsius
    */
   float voltageToCelsius(float voltage) {
      return 25 - (voltage-0.719)/.001715;
   }

public:
   using MovingWindowAverage<WindowSize>::getAverage;

   /**
    * Returns the moving window average of the internal chip temperature
    *
    * @return Chip temperature in Celsius
    */
   virtual float getTemperature() {

      return voltageToCelsius(getAverage());
   }
};

#endif /* SOURCES_MOVINGWINDOWAVERAGE_H_ */
