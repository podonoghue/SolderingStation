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

class AdcAverage {
private:
   AdcAverage(const AdcAverage &other) = delete;
   AdcAverage(AdcAverage &&other) = delete;
   AdcAverage& operator=(const AdcAverage &other) = delete;
   AdcAverage& operator=(AdcAverage &&other) = delete;

protected:
   /// Last sample added
   int lastSample = 0;

public:
   AdcAverage() {}

   /**
    * Convert ADC value to ADC input voltage
    *
    * @return Value calculated in volts
    */
   static constexpr float convertToAdcVoltage(float adcValue) {
      // Convert ADC value to voltage
      return adcValue * (ADC_REF_VOLTAGE/ADConverter::getSingleEndedMaximum(ADC_RESOLUTION));
   }

   /**
    * Get the value of the last sample added
    *
    * @return Sample as integer
    */
   int getLastAdcSample() {
      return lastSample;
   }

   /**
    * Get the voltage of the last sample added
    *
    * @return Sample as integer
    */
   float getLastAdcVoltage() {
      return convertToAdcVoltage(getLastAdcSample());
   }

};

/**
 * Class representing a simple moving average for ADC values.
 *
 * @tparam WindowSize Number of samples to average over
 */
template<unsigned WindowSize>
class SimpleMovingAverage : protected AdcAverage {

private:
   /// Samples over entire window (circular buffer)
   int samples[WindowSize] = {0};

   /// Index into samples FIFO
   unsigned index = 0;

   /// Count of valid values in FIFO
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
      sum   = 0;
      count = 0;
      index = 0;
   }

   /**
    * Add ADC sample to window
    *
    * @param value Sample to add - replaces oldest value
    */
   void accumulate(int value) {

      lastSample = value;

      // Add new value
      sum += value;

      // Count values in buffer with limit
      if (count < WindowSize) {
         // Buffer not full just count sample
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
    * Return ADC samples averaged over window
    *
    * @return Sample average
    */
   float getAveragedAdcSamples() const {
      if (count == 0) {
         return 0;
      }
      // Calculate average over window
      return (float)sum/count;
   }

   /**
    * Return ADC voltage averaged over window
    *
    * @return Voltage average
    */
   float getAveragedAdcVoltage() const {
      return convertToAdcVoltage(getAveragedAdcSamples());
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
class MovingAverage : protected AdcAverage {

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
    * Add ADC sample to weighted average
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
    * Calculate the weighted average of the ADC samples
    *
    * @return Sample average
    */
   float getAveragedAdcSamples() const {
      return accumulator;
   }

   /**
    * Calculate the weighted average of the ADC sample voltages
    *
    * @return Voltage average
    */
   float getAveragedAdcVoltage() const {
      // Convert ADC samples averaged over window to voltage
      return convertToAdcVoltage(getAveragedAdcSamples());
   }
};

/**
 * Class representing dummy average for ADC values.
 *
 * Dummy A(i) = S(i)
 */
class DummyAverage : protected AdcAverage {

private:
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
      lastSample = value;
   }

   /**
    * Calculate the average of the ADC samples
    *
    * @return value calculated
    */
   int getAveragedAdcSamples() const {
      return lastSample;
   }

   /**
    * Calculate the average of the ADC sample voltages
    *
    * @return Voltage average
    */
   float getAveragedAdcVoltage() const {
      // Convert averaged ADC value to voltage
      return convertToAdcVoltage(getAveragedAdcSamples());
   }
};

// Three methods for averaging
//using AveragingMethod = SimpleMovingAverage<10>; // 10*10ms = even weights over 100ms average
using AveragingMethod = MovingAverage<20>; // 10*10ms = declining weights over 100ms average
//using AveragingMethod = DummyAverage;

class TemperatureAverage : public AveragingMethod {

public:
   /**
    * Returns the averaged temperature
    *
    * @return Temperature in Celsius
    */
   virtual float getTemperature() = 0;

   /**
    * Returns the temperature from the last sample
    *
    * @return Temperature in Celsius
    */
   virtual float getInstantTemperature() = 0;

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
    * Gets average resistance of sensor.
    * Only applicable to a sensor that is resistance based (PTC, NTC device).
    *
    * @return Resistance in ohms
    */
   virtual float getResistance() {
      return 0.0;
   }

   /**
    * Gets average thermocouple voltage
    * Only applicable to a sensor that is thermocouple based.
    *
    * @return Thermocouple voltage
    */
   virtual float getThermocoupleVoltage() {
      return 0.0;
   }

};

/**
 * Class representing an average customised for a NTC thermistor
 * MF58 10k B3950
 */
class ThermistorMF58Average : public TemperatureAverage {

private:

   /**
    * Converts ADC voltage to thermistor resistance
    *
    * @param voltage ADC voltage
    *
    * @return Thermistor resistance in ohms
    */
   static float convertAdcVoltageToNtcResistance(float voltage) {
      /// NTC measurement current
      constexpr float NTC_MEASUREMENT_CURRENT  = 237E-6; // <- measured. Nominally (0.617/3.3E3)+15e-6 ~ 202uA!

      /// Gain of NTC measurement amplifier - voltage follower
      constexpr float NTC_MEASUREMENT_GAIN   = 1.0;

      /// NTC measurement ratio ohms/volt i.e. converts ADC voltage to R
      constexpr float NTC_MEASUREMENT_RATIO   = 1/(NTC_MEASUREMENT_CURRENT*NTC_MEASUREMENT_GAIN);

      return NTC_MEASUREMENT_RATIO * voltage;
   }

   /**
    * Converts ADC voltage to temperature
    *
    * @param resistance Thermistor resistance in ohms
    *
    * @return Corresponding temperature in Celsius
    */
   static float convertAdcVoltageToCelsius(float voltage) {

      // Convert ADC voltage to thermistor resistance
      float resistance = convertAdcVoltageToNtcResistance(voltage);

      // Value from curve fitting see spreadsheet
      // Thermistor (ntc_mf58, 10k B3950) curve fit.ods
      constexpr float A_constant = 1.80554E-03; //1.29869E-03;
      constexpr float B_constant = 8.15458E-05; //1.89836E-04;
      constexpr float C_constant = 9.43826E-06; //3.45639E-06;

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

public:
   /**
    * Returns the averaged thermistor temperature
    *
    * @return  Thermistor temperature in Celsius
    */
   virtual float getTemperature() override {
      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the thermistor temperature from the last sample
    *
    * @return  Thermistor temperature in Celsius
    */
   virtual float getInstantTemperature() override {
      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }

   /**
    * Returns the average resistance of the thermistor.
    *
    * @return
    */
   virtual float getResistance() override {
      return convertAdcVoltageToNtcResistance(getAveragedAdcVoltage());
   }
};

/**
 * Class representing an average customised for a T12 thermocouple
 */
class ThermocoupleT12Average : public TemperatureAverage {

private:
   /**
    * Converts ADC voltage to thermocouple temperature
    *
    * @param voltage ADC voltage
    *
    * @return Temperature in Celsius
    */
   static float convertAdcVoltageToCelsius(float voltage) {

      // Convert ADC voltage to TC voltage
      voltage *= TC_MEASUREMENT_RATIO;

      // Curve fitting was done using mV
      voltage *= 1000;

      // Value from linear curve fitting see spreadsheet 250 to 455 range
      constexpr float Ah_constant = -17.75;
      constexpr float Bh_constant = 41.59;

      float temperature = Ah_constant + Bh_constant*voltage;

      return temperature;
   }

public:
   /**
    * Returns the averaged thermocouple temperature relative to the cold reference
    *
    * @return Thermocouple temperature in Celsius
    */
   virtual float getTemperature() override {
      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the thermocouple temperature relative to the cold reference for the last sample
    *
    * @return Thermocouple temperature in Celsius
    */
   virtual float getInstantTemperature() {
      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }

   /**
    * Gets average thermocouple voltage
    *
    * @return Thermocouple voltage
    */
   virtual float getThermocoupleVoltage() override {
      return getAveragedAdcVoltage() * TC_MEASUREMENT_RATIO;
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
   static float convertAdcVoltageToCelsius(float voltage) {
      return 25 - (voltage-0.719)/.001715;
   }

public:

   /**
    * Returns the averaged internal chip temperature
    *
    * @return Chip temperature in Celsius
    */
   virtual float getTemperature() override {

      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the temperature from the last sample
    *
    * @return Temperature in Celsius
    */
   virtual float getInstantTemperature() override {

      return convertAdcVoltageToCelsius(getLastAdcVoltage());
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
    * Converts a ADC voltage to thermistor resistance
    * The ADC voltage is from the thermocouple amplifier with 1/TC_MEASUREMENT_RATIO gain measuring
    * the thermistor voltage produced from a Thevenin equivalent Rt and Vt i.e. voltage divider.
    *
    * @param voltage  PTC voltage
    *
    * @return PTC resistance
    */
   static float convertAdcVoltageToPtcResistance(float voltage) {

      // Convert ADC voltage to thermistor voltage
      voltage *= TC_MEASUREMENT_RATIO;

      // Thevenin equivalent supplying the PTC
      constexpr float Rbias  = 10000.0;
      constexpr float Ropamp = 100000.0+1000.0;
      constexpr float Vt = 3.3*(Ropamp)/(Rbias+Ropamp);
      constexpr float Rt = (Rbias*Ropamp)/(Rbias+Ropamp);

      // Voltage divider
      return Rt/((Vt/voltage)-1);
   }

   /**
    * Converts a ADC voltage to thermistor temperature
    * The ADC voltage is from the thermocouple amplifier with 1/TC_MEASUREMENT_RATIO gain measuring
    * the thermistor voltage produced from a Thevenin equivalent Rt and Vt i.e. voltage divider.
    *
    * @param ADC voltage
    *
    * @return Temperature in Celsius
    */
   static float convertAdcVoltageToCelsius(float voltage) {

      // Convert ADC voltage to resistance
      float Rptc = convertAdcVoltageToPtcResistance(voltage);

      // Curve fit from PTC resistance -> Temperature
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

public:
   /**
    * Returns the averaged thermistor temperature
    *
    * @return Thermistor temperature in Celsius
    */
   virtual float getTemperature() override {
      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the thermistor temperature from the last sample
    *
    * @return Thermistor temperature in Celsius
    */
   virtual float getInstantTemperature() override {
      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }

   /**
    * Returns the averaged resistance of the thermistor.
    *
    * @return
    */
   virtual float getResistance() {
      return convertAdcVoltageToPtcResistance(getAveragedAdcVoltage());
   }
};

#endif /* SOURCES_AVERAGING_H_ */
