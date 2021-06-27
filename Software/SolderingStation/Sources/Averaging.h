/*
 * MovingWIndowAverage.h
 *
 *  Created on: 4 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_AVERAGING_H_
#define SOURCES_AVERAGING_H_

#include "hardware.h"
#include "Peripherals.h"

/**
 * Class representing a simple moving average for ADC values.
 *
 * @tparam WindowSize Number of samples to average over
 */
template<unsigned WindowSize>
class SimpleMovingAverage {

private:
   /// Samples over entire window (circular buffer)
   int samples[WindowSize] = {0};

   /// Index into samples
   unsigned index = 0;

   unsigned count = 0;

   /// Summation of samples
   int sum = 0;

   SimpleMovingAverage(const SimpleMovingAverage &other) = delete;
   SimpleMovingAverage(SimpleMovingAverage &&other) = delete;
   SimpleMovingAverage& operator=(const SimpleMovingAverage &other) = delete;
   SimpleMovingAverage& operator=(SimpleMovingAverage &&other) = delete;

public:
   /**
    * Constructor
    */
   SimpleMovingAverage() {}

   /**
    * Destructor
    */
   virtual ~SimpleMovingAverage() {}

   /**
    * Reset average
    */
   void reset() {
      count = 0;
      sum   = 0;
   }

   /**
    * Add ADC value to window
    *
    * @param value to add - replaces oldest value
    */
   void accumulate(int value) {

      // Add new value
      sum += value;

      // Count values in buffer with limit
      if (count < WindowSize) {
         count++;
      }

      // Remove oldest value
      sum -= samples[index];

      // Save new value
      samples[index++] = value;

      // Wrap buffer
      if (index >= WindowSize) {
         index = 0;
      }
   }

   /**
    * Return raw ADC value averaged over window
    *
    * @return value calculated over window
    */
   float getAverage() const {
      if (count == 0) {
         return 0;
      }
      // Calculate average over window
      return (float)sum/count;
   }

   /**
    * Return ADC voltage averaged over window
    *
    * @return value calculated over window
    */
   float getVoltageAverage() const {

      float voltage;

      // Calculate average over window
      voltage = getAverage();

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/ADConverter::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

/**
 * Class representing a modified moving average for ADC values.
 *
 * A(i) = (s(i) + (N-1)A(i-1))/N = S(i)/N + (N-1)s(i-1)/N + (N-1)(N-1)S(i-2)/N*N ...
 *
 * @tparam N is the weighting in above equation.
 */
template<unsigned N=2>
class MovingAverage {

private:
   /// Sample accumulator
   float accumulator = 0;
   bool initial = true;

   MovingAverage(const MovingAverage &other) = delete;
   MovingAverage(MovingAverage &&other) = delete;
   MovingAverage& operator=(const MovingAverage &other) = delete;
   MovingAverage& operator=(MovingAverage &&other) = delete;

public:
   /**
    * Constructor
    */
   MovingAverage() {}

   /**
    * Destructor
    */
   virtual ~MovingAverage() {}

   /**
    * Reset average
    */
   void reset() {
      accumulator = 0;
      initial     = true;
   }

   /**
    * Add ADC value to weighted average
    *
    * @param value to add
    */
   void accumulate(int value) {
      if (initial) {
         accumulator = value;
         initial     = false;
      }
      else {
         accumulator = ((N-1)*accumulator + value)/N;
      }
   }

   /**
    * Return raw ADC value as weighted average
    *
    * @return value calculated
    */
   float getAverage() const {
      return accumulator;
   }

   /**
    * Return ADC voltage as weighted averaged
    *
    * @return value calculated
    */
   float getVoltageAverage() const {

      float voltage;

      // Calculate average over window
      voltage = getAverage();

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/ADConverter::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

/**
 * Class representing dummy average for ADC values.
 *
 * Dummy A(i) = S(i)
 */
class DummyAverage {

private:
   /// Samples
   int accumulator = 0;

   DummyAverage(const DummyAverage &other) = delete;
   DummyAverage(DummyAverage &&other) = delete;
   DummyAverage& operator=(const DummyAverage &other) = delete;
   DummyAverage& operator=(DummyAverage &&other) = delete;

public:
   /**
    * Constructor
    */
   DummyAverage() {}

   /**
    * Destructor
    */
   virtual ~DummyAverage() {}

   /**
    * Reset average
    */
   void reset() {
   }

   /**
    * Add ADC value to weighted average
    *
    * @param value to add
    */
   void accumulate(int value) {
      accumulator = value;
   }

   /**
    * Return raw ADC value as weighted average
    *
    * @return value calculated
    */
   int getAverage() const {
      return accumulator;
   }
   /**
    * Return ADC voltage as weighted averaged
    *
    * @return value calculated
    */
   float getVoltageAverage() const {

      float voltage;

      // Calculate average over window
      voltage = getAverage();

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/ADConverter::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

// Three methods for averaging
//using AveragingMethod = SimpleMovingAverage<10>; // 20*100ms = 2s average
//using AveragingMethod = MovingAverage<5>;
using AveragingMethod = DummyAverage;

/**
 * Class representing a average customised for a thermistor
 *
 * @tparam WindowSize Number of samples to average over
 */
class ThermistorAverage : public AveragingMethod {

private:

   /**
    * Converts thermistor resistance value to temperature
    *
    * @param resistance Thermistor resistance in ohms
    *
    * @return Corresponding temperature in Celsius
    */
   static float resistanceToCelsius(float resistance) {

      // Value from curve fitting see spreadsheet
      constexpr float A_constant =  1.62770581419817E-03;
      constexpr float B_constant =  6.39056547750702E-05;
      constexpr float C_constant =  1.91419439391882E-05;
      constexpr float D_constant = -6.36504328625315E-07;

      constexpr float KelvinToCelsius = -274.15;

      // Used to calculate R^N
      float log_R_Nth = log(resistance);

      float power;

      // Used to calculate 1/T in Kelvin
      float reciprocalTemperature;

      reciprocalTemperature  = A_constant;
      power  = log_R_Nth;
      reciprocalTemperature += B_constant * power;
      power *= log_R_Nth;
      reciprocalTemperature += C_constant * power;
      power *= log_R_Nth;
      reciprocalTemperature += D_constant * power;

      float temperatureInKelvin = 1/reciprocalTemperature;

      return temperatureInKelvin + KelvinToCelsius;
   }

public:
   /**
    * Returns the average of the thermistor resistance
    *
    * @return Thermistor resistance in ohms
    */
   float getResistance() {

      // Get ADC value as voltage
      float voltage = getVoltageAverage();

      return NTC_MEASUREMENT_RATIO * voltage;
   }

   /**
    * Returns the average of the thermistor temperature in Celsius
    *
    * @return  Thermistor temperature in Celsius
    */
   float getTemperature() {

      return resistanceToCelsius(getResistance());
   }
};

/**
 * Class representing an average customised for a thermocouple
 */
class ThermocoupleAverage : public AveragingMethod {

private:

   /**
    * Converts a thermocouple voltage to temperature
    *
    * @param voltage Thermocouple voltage
    *
    * @return Temperature in Celsius
    */
   static float voltageToCelsius(float voltage) {

      // Curve fitting was done using mV
      voltage *= 1000;

      // Value from linear curve fitting see spreadsheet 250 to 455 range
      constexpr float Ah_constant = -17.75 ;//15.25; //-20.3057343295616; // -12.9509284993216;
      constexpr float Bh_constant = 41.59; //38.48; // 46.191866056261;  //45.4204980163692;

      float temperature = Ah_constant + Bh_constant*voltage;

      return temperature;
   }

public:
   /**
    * Returns the average of the thermocouple voltage
    *
    * @return Thermocouple voltage in V
    */
   float getVoltage() {

      // Get ADC value as voltage
      float voltage = getVoltageAverage();

      return voltage * TC_MEASUREMENT_RATIO;
   }

   /**
    * Returns the average of the thermocouple temperature relative to the cold reference
    *
    * @return Thermocouple temperature in Celsius
    */
   virtual float getTemperature() {

      return voltageToCelsius(getVoltage());
   }
};

/**
 * Class representing an average customised for the internal temperature sensor
 */
class ChipTemperatureAverage : public AveragingMethod {

private:

   /**
    * Convert ADC voltage measurement to Internal chip temperature
    *
    * @param voltage measurement
    *
    * @return Chip temperature in Celsius
    */
   static float voltageToCelsius(float voltage) {
      return 25 - (voltage-0.719)/.001715;
   }

public:

   /**
    * Returns the average of the internal chip temperature
    *
    * @return Chip temperature in Celsius
    */
   virtual float getTemperature() {

      return voltageToCelsius(getVoltageAverage());
   }
};

#endif /* SOURCES_AVERAGING_H_ */
