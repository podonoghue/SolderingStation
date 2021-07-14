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

   /// Last sample added
   int lastSample = 0;

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
      index = 0;
   }

   /**
    * Add ADC value to window
    *
    * @param value to add - replaces oldest value
    */
   void accumulate(int value) {

      lastSample = value;

      // Add new value
      sum += value;

      // Count values in buffer with limit
      if (count < WindowSize) {
         // Buffer not full just count
         count++;
      }
      else {
         // Now acting as circular buffer - remove oldest value
         sum -= samples[index];
      }

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
    * Get the value of the last sample added
    *
    * @return Sample as integer
    */
   int getLastSample() {
      return lastSample;
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

   /// Last sample added
   int lastSample = 0;

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

      lastSample = value;

      if (initial) {
         accumulator = value;
         initial     = false;
      }
      else {
         accumulator = ((N-1)*accumulator + value)/N;
      }
   }

   /**
    * Get the value of the last sample added
    *
    * @return Sample as integer
    */
   int getLastSample() {
      return lastSample;
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
    * Get the value of the last sample added
    *
    * @return Sample as integer
    */
   int getLastSample() {
      return accumulator;
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
//using AveragingMethod = SimpleMovingAverage<10>; // 10*10ms = even weights over 100ms average
using AveragingMethod = MovingAverage<10>; // 10*10ms = declining weights over 100ms average
//using AveragingMethod = DummyAverage;

class TemperatureAverage : public AveragingMethod {

public:
   virtual float getTemperature() = 0;

   /**
    * Indicates if the measurement requires a bias current.
    * This is needed when the thermocouple amplifier is used to measure a low-value PTC rather than a TC.
    *
    * @return
    */
   virtual bool isBiasRequired()  {
      return false;
   }
   /**
    * Calculates resistance of sensor.
    * Only applicable to a sensor that is resistance based (PTC,NTC device).
    *
    * @return
    */
   virtual float getResistance() {
      return 0.0;
   }

};

/**
 * Class representing an average customised for a NTC thermistor
 * MF58 10k B3950
 */
class ThermistorMF58Average : public TemperatureAverage {

protected:

   /**
    * Converts thermistor resistance value to temperature
    *
    * @param resistance Thermistor resistance in ohms
    *
    * @return Corresponding temperature in Celsius
    */
   static float resistanceToCelsius(float resistance) {

      // Value from curve fitting see spreadsheet
      // Thermistor (ntc_mf58, 10k B3950) curve fit.ods
      constexpr float A_constant =  1.29869E-03;
      constexpr float B_constant =  1.89836E-04;
      constexpr float C_constant =  3.45639E-06;

      constexpr float KelvinToCelsius = -274.15;

      // Used to calculate log(R)^N
      float log_R_Nth = log(resistance);

      float power;

      // Used to calculate 1/T in Kelvin
      float reciprocalTemperature;

      reciprocalTemperature  = A_constant;
      power  = log_R_Nth;
      reciprocalTemperature += B_constant * power;
      power *= log_R_Nth;
      reciprocalTemperature += C_constant * power;

      float temperatureInKelvin = 1/reciprocalTemperature;

      return temperatureInKelvin + KelvinToCelsius;
   }

   /**
    * Returns the average of the thermistor resistance
    *
    * @return Thermistor resistance in ohms
    */
   float getResistance() {
      /// NTC measurement current
      constexpr float NTC_MEASUREMENT_CURRENT  = 237E-6; // <- measured. Nominally (0.617/3.3E3)+15e-6 ~ 202uA!

      /// Gain of NTC measurement amplifier - voltage follower
      constexpr float NTC_MEASUREMENT_GAIN   = 1.0;

      /// NTC measurement ratio ohms/volt i.e. converts ADC voltage to R
      constexpr float NTC_MEASUREMENT_RATIO   = 1/(NTC_MEASUREMENT_CURRENT*NTC_MEASUREMENT_GAIN);

      // Get ADC value as voltage
      float voltage = getVoltageAverage();

      return NTC_MEASUREMENT_RATIO * voltage;
   }

public:
   /**
    * Returns the average of the thermistor temperature in Celsius
    *
    * @return  Thermistor temperature in Celsius
    */
   virtual float getTemperature() override {

      return resistanceToCelsius(getResistance());
   }
};

/**
 * Class representing an average customised for a thermocouple
 */
class ThermocoupleAverage : public TemperatureAverage {

protected:
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
      constexpr float Ah_constant = -17.75;
      constexpr float Bh_constant = 41.59;

      float temperature = Ah_constant + Bh_constant*voltage;

      return temperature;
   }

   /**
    * Returns the average of the thermocouple voltage
    *
    * @return Thermocouple voltage in V
    */
   float getVoltage() {
      /// Thermocouple measurement ratio V/V i.e. converts ADC voltage to thermocouple voltage in V
      /// Amplifier gain is Rf/Ri
      constexpr float TC_MEASUREMENT_RATIO   = (Ri/Rf);

      // Get ADC value as voltage
      float voltage = getVoltageAverage();

      return voltage * TC_MEASUREMENT_RATIO;
   }

public:
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
class ChipTemperatureAverage : public TemperatureAverage {

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
   virtual float getTemperature() override {

      return voltageToCelsius(getVoltageAverage());
   }
};

/**
 * Class representing an dummy average - always zero
 */
class ZeroAverage : public TemperatureAverage {

public:
   virtual float getTemperature() override {
      return 0.0;
   }
};
/**
 * Class representing an average customised for a Weller PTC Thermistor
 */
class WellerThermistorAverage : public TemperatureAverage {

private:
   /**
    * Convert PTC thermistor voltage measurement to PTC resistance
    *
    * @param voltage  PTC voltage
    *
    * @return PTC resistance
    */
   static float getPtcResistance(float voltage) {
      // Thevenin equivalent supplying the PTC
      constexpr float Rbias  = 10000.0;
      constexpr float Ropamp = 100000.0+1000.0;
      constexpr float Vt = 3.3*(Ropamp)/(Rbias+Ropamp);
      constexpr float Rt = (Rbias*Ropamp)/(Rbias+Ropamp);

      // Voltage divider
      return Rt/((Vt/voltage)-1);
   }

   /**
    * Converts a PTC thermistor voltage to temperature
    * The voltage is produced by a voltage divider with Thevenin equivalent Rt and Vt.
    *
    * @param voltage PTC voltage
    *
    * @return Temperature in Celsius
    */
   static float voltageToCelsius(float voltage) {

      float Rptc = getPtcResistance(voltage);

      // Linear interpolation from R -> T
      constexpr float A_constant =  -79.560;
      constexpr float B_constant =   -0.994;
      constexpr float C_constant =    0.243;

      float temperature  = A_constant;
      float power = Rptc;
      temperature += B_constant * power;
      power *= Rptc;
      temperature += C_constant * power;

//      USBDM::console.write("Vptc=").write(1000*voltage).write("mV Rptc=").write(Rptc).write(" T= ").writeln(temperature);

      return temperature;
   }

   /**
    * Returns the average of the thermistor voltage
    *
    * @return thermistor voltage in V
    */
   float getVoltage() {
      // Sensor measurement ratio V/V i.e. converts ADC voltage to sensor voltage in V
      // Amplifier gain is Rf/Ri = 100K/1K
      constexpr float TC_MEASUREMENT_RATIO   = (Ri/Rf);

      // Get ADC value as voltage
      float voltage = getVoltageAverage();

      return voltage * TC_MEASUREMENT_RATIO;
   }

public:
   /**
    * Returns the average of the thermistor temperature
    *
    * @return Thermistor temperature in Celsius
    */
   virtual float getTemperature() override {
      return voltageToCelsius(getVoltage());
   }

   /**
    * Returns the average resistance of thermistor.
    *
    * @return
    */
   virtual float getResistance() {
      return getPtcResistance(getVoltage());
   }
};


#endif /* SOURCES_AVERAGING_H_ */
