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
#include "stringFormatter.h"

class T12 : public Measurement {

private:
   /// Thermistor in handle
   ThermistorMF58Average coldJunctionThermistor;

   /// Thermocouple in cartridge
   ThermocoupleT12Average   thermocouple;

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
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() override {
      return thermocouple.getInstantTemperature()+coldJunctionThermistor.getTemperature();
   }

   /**
    * Set calibration data from current measurements
    *
    * @param calibrationIndex Index for the calibration
    * @param tipSettings      Tip-settings to update
    */
   virtual void setCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) override {
      float thermocoupleVoltage  = thermocouple.getThermocoupleVoltage();
      float coldJunctionTemp     = coldJunctionThermistor.getTemperature();
      USBDM::console.write(calibrationIndex);
      USBDM::console.write(" : TC = ").write(1000*thermocoupleVoltage);
      USBDM::console.write("mV, Cold = ").writeln(coldJunctionTemp);
      tipSettings.setCalibrationPoint(calibrationIndex, thermocoupleVoltage, coldJunctionTemp);
   }

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @note Not re-entrant!
    *
    * @return Point to static string
    */
   virtual const char *reportCalibrationValues() override {
      getStringFormatter().clear();
      float thermocoupleVoltage  = thermocouple.getThermocoupleVoltage();
      float coldJunctionTemp     = coldJunctionThermistor.getTemperature();
      getStringFormatter().setFloatFormat(1);
      getStringFormatter().write("Vtc=").write(1000*thermocoupleVoltage);
      getStringFormatter().write("mV Tcj=").write(coldJunctionTemp).write('C');
      return getStringFormatter().toString();
   }

   /**
    * Set set default calibration values
    *
    * @param tipNameIndex  Tip name index for this setting
    */
   static void initialiseSettings(TipSettings *settings, const InitialTipInfo &) {

      // Dummy values
      settings->setCalibrationPoint(CalibrationIndex_250, 25, 5.4);
      settings->setCalibrationPoint(CalibrationIndex_250, 25, 5.4);
      settings->setCalibrationPoint(CalibrationIndex_250, 25, 5.4);

      settings->setInitialPidControlValues(10.0,0.2,0.0,20.0);
   }

   virtual float getMeasurement() override {
      return coldJunctionThermistor.getResistance();
   };

};

#endif /* T12_H_ */
