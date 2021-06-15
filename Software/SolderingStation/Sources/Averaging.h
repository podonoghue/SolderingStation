/*
 * MovingWIndowAverage.h
 *
 *  Created on: 4 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_MOVINGWINDOWAVERAGE_H_
#define SOURCES_MOVINGWINDOWAVERAGE_H_

#include "hardware.h"
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
   int samples[WindowSize] = {0};

   /// Index into samples
   unsigned index = 0;

   /// Summation of samples
   int sum = 0;

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
   void accumulate(int value) {

      sum += value;
      sum -= samples[index];
      samples[index++] = value;
      if (index >= WindowSize) {
         index = 0;
      }
   }

   /**
    * Return raw ADC value averaged over window
    *
    * @return value calculated over window
    */
   int getAverage() {

      // Calculate average over window
      return round((float)sum/WindowSize);
   }
   /**
    * Return ADC voltage averaged over window
    *
    * @return value calculated over window
    */
   float getVoltageAverage() {

      float voltage;

      // Calculate average over window
      voltage = (float)sum/WindowSize;

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/USBDM::Adc0::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

/**
 * Class representing a moving average for ADC values.
 *
 * A(i) = (s(i) + A(i-1))/2 = S(1)/2 + s(i-1)/4 + S(i-2)/8 ...
 */
class MovingAverage {

private:
   /// Samples over entire window (circular buffer)
   int accumulator = 0;

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
    * Add ADC value to weighted average
    *
    * @param value to add
    */
   void accumulate(int value) {
      accumulator = (accumulator + value)/2;
   }

   /**
    * Return raw ADC value as weighted average
    *
    * @return value calculated
    */
   int getAverage() {
      return accumulator;
   }
   /**
    * Return ADC voltage as weighted averaged
    *
    * @return value calculated
    */
   float getVoltageAverage() {

      float voltage;

      // Calculate average over window
      voltage = getAverage();

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/USBDM::Adc0::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

/**
 * Class representing a moving average for ADC values.
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
   int getAverage() {
      return accumulator;
   }
   /**
    * Return ADC voltage as weighted averaged
    *
    * @return value calculated
    */
   float getVoltageAverage() {

      float voltage;

      // Calculate average over window
      voltage = getAverage();

      // Convert ADC value to voltage
      voltage  *= (ADC_REF_VOLTAGE/USBDM::Adc0::getSingleEndedMaximum(ADC_RESOLUTION));

      return voltage;
   }
};

//using AveragingMethod = MovingWindowAverage<5>;
using AveragingMethod = MovingAverage;
//using AveragingMethod = DummyAverage;

/**
 * Class representing a moving average window customised for a thermistor
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
   float resistanceToCelsius(float resistance){

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
    * Returns the moving window average of the thermistor resistance
    *
    * @return Thermistor resistance in ohms
    */
   float getResistance() {

      // Get ADC value as voltage
      float voltage = getVoltageAverage();

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
class ThermocoupleAverage : public AveragingMethod {

private:

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

      // Value from linear curve fitting see spreadsheet 250 to 455 range
      constexpr float Ah_constant = 15.25; //-20.3057343295616; // -12.9509284993216;
      constexpr float Bh_constant = 38.48; // 46.191866056261;  //45.4204980163692;

      float temperature = Ah_constant + Bh_constant*voltage;

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
      float voltage = getVoltageAverage();

      return voltage * TC_MEASUREMENT_RATIO;
   }

   /**
    * Returns the moving window average of the thermocouple temperature relative to the cold reference
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
class ChipTemperatureAverage : public AveragingMethod {

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

   /**
    * Returns the moving window average of the internal chip temperature
    *
    * @return Chip temperature in Celsius
    */
   virtual float getTemperature() {

      return voltageToCelsius(getVoltageAverage());
   }
};

#endif /* SOURCES_MOVINGWINDOWAVERAGE_H_ */
