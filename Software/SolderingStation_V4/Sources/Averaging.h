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
#include "NonvolatileSettings.h"

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
   int getLastAdcSample() const {
      return lastSample;
   }

   /**
    * Get the voltage of the last sample added
    *
    * @return Sample as integer
    */
   float getLastAdcVoltage() const {
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
using AveragingMethod = MovingAverage<10>; // 10*10ms = declining weights over 100ms average
//using AveragingMethod = DummyAverage;

class TemperatureAverage : public AveragingMethod {

public:
   /**
    * Returns the averaged temperature
    *
    * @return Temperature in Celsius
    */
   virtual float getTemperature() const = 0;

   /**
    * Returns the temperature from the last sample
    *
    * @return Temperature in Celsius
    */
   virtual float getInstantTemperature() const = 0;

   /**
    * Gets average resistance of sensor.
    * Only applicable to a sensor that is resistance based (PTC, NTC device).
    *
    * @return Resistance in ohms
    */
   virtual float getResistance() const {
      return 0.0;
   }

   /**
    * Gets average thermocouple voltage
    * Only applicable to a sensor that is thermocouple based.
    *
    * @return Thermocouple voltage
    */
   virtual float getThermocoupleVoltage() const {
      return 0.0;
   }

};

/**
 * Class representing an average customised for a NTC thermistor
 * MF58 10k B3950
 */
class ThermistorMF58Average : public TemperatureAverage {

private:

   /// Measurement path being used
   static constexpr MuxSelect MEASUREMENT = MuxSelect_LowGainBiased;

   /**
    * Converts ADC voltage to thermistor resistance
    *
    * @param voltage ADC voltage
    *
    * @return Thermistor resistance in ohms
    */
   float convertAdcVoltageToNtcResistance(float voltage) const {

      /// Gain of measurement path
      const float gain  = nvinit.hardwareCalibration.lowGainNoBoost;

      if (voltage>2.99) {
         // Assume ADC at maximum => open resistor
         return std::nanf("");//std::numeric_limits<float>::max();
      }

      // Scale to input voltage (voltage at divider)
      voltage *= gain;

      return BIAS_RESISTOR_VALUE / ((BIAS_VOLTAGE/voltage) - 1);
   }

   /**
    * Converts ADC voltage to temperature
    *
    * @param resistance Thermistor resistance in ohms
    *
    * @return Corresponding temperature in Celsius
    */
   float convertAdcVoltageToCelsius(float voltage) const {

      // Convert ADC voltage to thermistor resistance
      float resistance = convertAdcVoltageToNtcResistance(voltage);

      // Value from curve fitting see spreadsheet
      // Sensor curve fitting.ods
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
   virtual float getTemperature() const override {
      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the thermistor temperature from the last sample
    *
    * @return  Thermistor temperature in Celsius
    */
   virtual float getInstantTemperature() const override {
      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }

   /**
    * Returns the average resistance of the thermistor.
    *
    * @return
    */
   virtual float getResistance() const override {
      return convertAdcVoltageToNtcResistance(getAveragedAdcVoltage());
   }

   /**
    * Returns the measurement mux setting (excluding channel) to use
    *
    * @return
    */
   static constexpr MuxSelect getMeasurement() {
      return MEASUREMENT;
   }
};

/**
 * Class representing an average customised for a thermocouple
 */
class ThermocoupleAverage : public TemperatureAverage {
private:
   // Temperature calibration values
   float calibrationTemperatures[3];

   // Thermocouple voltage calibration values
   float calibrationVoltages[3];

   /// Measurement path being used
   static constexpr MuxSelect MEASUREMENT = MuxSelect_HighGainBoost;

public:

   /**
    * Converts ADC voltage to thermocouple voltage
    * This accounts for the sampling and amplifier chain.
    *
    * @param voltage ADC voltage
    *
    * @return Thermocouple voltage
    */
   static float convertAdcVoltageToThermocoupleVoltage(float voltage) {

      /// Gain of measurement path
      const float gain  = nvinit.hardwareCalibration.highGainWithBoost;

      return voltage * gain;
   }

   /**
    * Converts ADC voltage to thermocouple relative temperature
    *
    * @param voltage ADC voltage
    *
    * @return Temperature in Celsius
    */
   float convertAdcVoltageToCelsius(float voltage) const {

      // Convert ADC voltage to TC voltage
      voltage = convertAdcVoltageToThermocoupleVoltage(voltage);

      // Calibration fitting was done using mV
      voltage *= 1000;

      // Simple linear interpolation with three calibration points
      float lastV = 0.0;
      float lastT = 0.0;
      unsigned index;
      for (index=0; index<2; index++) {
         if (voltage < calibrationVoltages[index]) {
            break;
         }
         lastV = calibrationVoltages[index];
         lastT = calibrationTemperatures[index];
      }
      float temperature = lastT + ((calibrationTemperatures[index]-lastT)*(voltage-lastV)/(calibrationVoltages[index]-lastV));

//      // Value from linear curve fitting see spreadsheet 250 to 455 range
//      constexpr float Ah_constant = -17.75;
//      constexpr float Bh_constant = 41.59;
//
//      float temperature = Ah_constant + Bh_constant*voltage;

      return temperature;
   }

public:
   /**
    * Returns the averaged thermocouple temperature relative to the cold reference
    *
    * @return Thermocouple temperature in Celsius
    */
   virtual float getTemperature() const override {
      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the thermocouple temperature relative to the cold reference for the last sample
    *
    * @return Thermocouple temperature in Celsius
    */
   virtual float getInstantTemperature() const override {
      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }

   /**
    * Gets average thermocouple voltage
    *
    * @return Thermocouple voltage
    */
   virtual float getThermocoupleVoltage() const override {
      return convertAdcVoltageToThermocoupleVoltage(getAveragedAdcVoltage());
   }

   /**
    * Set calibration values for thermocouple
    *
    * @param tipSettings value1 = Vt, value2 = Tt
    */
   void setCalibrationValues(const TipSettings *tipSettings) {
      for (CalibrationIndex index=CalibrationIndex_250; index<=CalibrationIndex_400; ++index) {
         calibrationTemperatures[index] = tipSettings->getCalibrationTempValue(index);
         calibrationVoltages[index]     = tipSettings->getCalibrationMeasurementValue(index);
      }
   }

   /**
    * Returns the measurement mux setting (excluding channel) to use
    *
    * @return
    */
   static constexpr MuxSelect getMeasurement() {
      return MEASUREMENT;
   }
};

/**
 * Class representing an average customised for the chip internal temperature sensor
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
   virtual float getTemperature() const override {

      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the temperature from the last sample
    *
    * @return Temperature in Celsius
    */
   virtual float getInstantTemperature() const override {

      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }
};

/**
 * Class representing an dummy average - always zero
 */
class ZeroAverage : public TemperatureAverage {

public:
   virtual float getTemperature() const override {
      return 0.0;
   }
};

/**
 * Class representing an average customised for a Weller PTC Thermistor
 */
class WellerThermistorAverage : public TemperatureAverage {

private:

   /// Measurement path being used
   static constexpr MuxSelect MEASUREMENT = MuxSelect_HighGainBoostBiased;

   // Temperature calibration values
   float calibrationTemperatures[3];

   // Thermistor resistance calibration values
   float calibrationResistances[3];

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

      /// Gain of measurement path
      const float gain  = nvinit.hardwareCalibration.highGainWithBoost;

      if (voltage>2.99) {
         // Assume ADC at maximum => open resistor
         return std::nanf("");
      }

      // Scale to input voltage (voltage at divider)
      voltage *= gain;

      // Voltage divider
      return BIAS_RESISTOR_VALUE / ((BIAS_VOLTAGE/voltage) - 1);
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
   float convertAdcVoltageToCelsius(float voltage) const {

      // Convert ADC voltage to resistance
      float Rptc = convertAdcVoltageToPtcResistance(voltage);

      // Simple linear interpolation with three calibration points
      float lastR = 22.0;
      float lastT = 0.0;
      unsigned index;
      for (index=0; index<2; index++) {
         if (Rptc < calibrationResistances[index]) {
            break;
         }
         lastR = calibrationResistances[index];
         lastT = calibrationTemperatures[index];
      }
      float temperature = lastT + ((calibrationTemperatures[index]-lastT)*(Rptc-lastR)/(calibrationResistances[index]-lastR));

#if 0
      // Curve fit from PTC resistance -> Temperature
      constexpr float A_constant =  -79.560;
      constexpr float B_constant =   -0.994;
      constexpr float C_constant =    0.243;

      float temperature  = A_constant;
      float power = Rptc;
      temperature += B_constant * power;
      power *= Rptc;
      temperature += C_constant * power;
#endif

      return temperature;

//      USBDM::console.write("Vptc=").write(1000*voltage).write("mV Rptc=").write(Rptc).write(" T= ").writeln(temperature);

      return temperature;
   }

public:
   /**
    * Returns the averaged thermistor temperature
    *
    * @return Thermistor temperature in Celsius
    */
   virtual float getTemperature() const override {
      return convertAdcVoltageToCelsius(getAveragedAdcVoltage());
   }

   /**
    * Returns the thermistor temperature from the last sample
    *
    * @return Thermistor temperature in Celsius
    */
   virtual float getInstantTemperature() const override {
      return convertAdcVoltageToCelsius(getLastAdcVoltage());
   }

   /**
    * Returns the averaged resistance of the thermistor.
    *
    * @return
    */
   virtual float getResistance() const {
      return convertAdcVoltageToPtcResistance(getAveragedAdcVoltage());
   }

   /**
    * Set calibration values for thermistor
    *
    * @param tipsettings
    */
   void setCalibrationValues(const TipSettings *tipsettings) {
      for (CalibrationIndex index = CalibrationIndex_250; index <= CalibrationIndex_400; ++index) {
         calibrationResistances[index]  = tipsettings->getCalibrationMeasurementValue(index);
         calibrationTemperatures[index] = tipsettings->getCalibrationTempValue(index);
      }
   }

   /**
    * Returns the measurement mux setting (excluding channel) to use
    *
    * @return
    */
   static constexpr MuxSelect getMeasurement() {
      return MEASUREMENT;
   }
};

#endif /* SOURCES_AVERAGING_H_ */
