/*
 * T12.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef T12_H_
#define T12_H_

#include "stringFormatter.h"

#include "Measurement.h"
#include "Averaging.h"
#include "PidController.h"

/**
 * Class representing information for a T12 soldering iron
 */
class T12 : public Measurement {

private:

   /// Thermocouple in cartridge
   ThermocoupleAverage   thermocouple;

   /// Thermistor in handle
   ThermistorMF58Average coldJunctionThermistor;

   /// Measurement to use for thermocouple on sub-channel A
   static constexpr MuxSelect Measurement1_ChaThermocouple = muxSelectAddSubChannel(ThermocoupleAverage::getMeasurement(), SubChannelNum_A);

   /// Measurement to use for thermistor on sub_channel B
   static constexpr MuxSelect Measurement2_ChbColdRef      = muxSelectAddSubChannel(ThermistorMF58Average::getMeasurement(), SubChannelNum_B);

   /// Loop controller
   PidController controller{PID_INTERVAL, MIN_DUTY, MAX_DUTY};

public:
   T12(Channel &ch) : Measurement(ch, 8.5, 24) {}

   virtual ~T12() {}

   /**
    * Enable control loop
    *
    * @param[in] enable True to enable
    */
   virtual void enableControlLoop(bool enable) override {
      controller.enable(enable);
   }

   /**
    * Run end of cycle update:
    *   - Drive state
    *   - Temperature averages
    *   - Power average
    *   - Controller iteration
    */
   virtual uint8_t update(float targetTemperature) override {

      // Update drive to heaters as needed
      controller.advance();

      // Update power average (as percentage)
      power.accumulate(controller.getDutyCycle());

      // Run PID

      float dc = controller.newSample(targetTemperature, getTemperature());
#if 0
      // Safety check
      // Turn off after 20s at >20% drive
      static unsigned highOnTime = 0;
      if (dc < 20) {
         highOnTime = 0;
      }
      highOnTime++;
      if (highOnTime>20/PID_INTERVAL) {
         dc = 0;
      }
#endif
      controller.setDutyCycle(dc);

      return controller.isOn()?0b11:0b00;
   }

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() const override {
      return thermocouple.getTemperature()+coldJunctionThermistor.getTemperature();
   }

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() const override {
      return thermocouple.getInstantTemperature()+coldJunctionThermistor.getTemperature();
   }

   /**
    * Set calibration data from current measurements
    *
    * @param calibrationIndex Index for the calibration
    * @param tipSettings      Tip-settings to update
    *
    * @return true  - Success
    * @return false - Fail - values appear out of range
    */
   virtual bool saveCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) override {
      using namespace USBDM;

      float thermocoupleVoltage_mV  = 1000*thermocouple.getThermocoupleVoltage();
      float coldJunctionTemp        = coldJunctionThermistor.getTemperature();

      console.write(calibrationIndex);
      console.write(" : TC = ").write(thermocoupleVoltage_mV);
      console.write("mV, Cold = ").writeln(coldJunctionTemp);

      if ((thermocoupleVoltage_mV < 4) || (thermocoupleVoltage_mV > 12)) {
         return false;
      }
      if ((coldJunctionTemp < 0) || (coldJunctionTemp > 60)) {
         return false;
      }

      float thermocoupleTemperature = TipSettings::getCalibrationTemperature(calibrationIndex) - coldJunctionTemp;
      tipSettings.setCalibrationPoint(calibrationIndex, thermocoupleTemperature, thermocoupleVoltage_mV);

      return true;
   }

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @param io      Where to write report to
    * @param brief   True to shorten report (2 lines maximum)
    */
   virtual void reportCalibrationValues(USBDM::FormattedIO &io, bool brief) const override {
      float thermocoupleVoltage  = thermocouple.getThermocoupleVoltage();
      float thermocoupleTemp     = thermocouple.getTemperature();
      float coldJunctionTemp     = coldJunctionThermistor.getTemperature();
      float thermisterResistance = coldJunctionThermistor.getResistance();

      io.
         setFloatFormat(1, USBDM::Padding_LeadingSpaces, 3).
         write("TC=").write(thermocoupleTemp).write(" C (").
         setFloatFormat(1).
         write(1000*thermocoupleVoltage).writeln(" mV)");
      io.
         setFloatFormat(1, USBDM::Padding_LeadingSpaces, 3).
         write("CJ=").write(coldJunctionTemp).write(" C (").
         setFloatFormat(2).
         write(thermisterResistance/1000).writeln(" K)");
      if (!brief) {
         io.
            setFloatFormat(1, USBDM::Padding_LeadingSpaces, 3).
            write("T =").write(coldJunctionTemp+thermocoupleTemp).write(" C");
      }
   }

   /**
    * Get default calibration and control values
    *
    * @param settings
    * @param initInfo
    */
   static void initialiseSettings(TipSettings *settings, const InitialTipInfo &initInfo) {

      (void)initInfo;
      // Dummy values                                      Tt(C)    Vt(mV)
      settings->setCalibrationPoint(CalibrationIndex_250,  221.77,  4.64); // 5.758
      settings->setCalibrationPoint(CalibrationIndex_325,  296.06,  5.81); // 7.546
      settings->setCalibrationPoint(CalibrationIndex_400,  369.61,  6.64); // 8.974

      settings->setInitialPidControlValues(10.0,0.2,0.0,20.0);
   }

   /**
    * Set calibration data
    *
    * @param[in] tipsettings Settings object with calibration data
    */
   virtual void setCalibrationValues(const TipSettings *tipsettings) override {
      thermocouple.setCalibrationValues(tipsettings);
      controller.setControlParameters(tipsettings);
   }

   /**
    * Get the sequence of ADC measurements to do
    *
    * @return Sequence array terminated by MuxSelect_Complete sentinel
    */
   virtual MuxSelect const *getMeasurementSequence() const override {
      static const MuxSelect sequence[] = {
            Measurement1_ChaThermocouple,
            Measurement2_ChbColdRef,
            MuxSelect_Complete,
      };
//      return sequence;
      return tipPresent?sequence:sequence+1;
   };

   /**
    * Process ADC value
    *
    * @param[in] muxSelect  Indicates measurement made
    * @param[in] adcValue   ADC value from measurement
    */
   virtual void processMeasurement(MuxSelect muxSelect, uint32_t adcValue) override {
      (void) adcValue;
      switch(muxSelect) {

         case Measurement1_ChaThermocouple:
            // Thermocouple
            thermocouple.accumulate(adcValue);
            break;

         case Measurement2_ChbColdRef:
            // NTC
            coldJunctionThermistor.accumulate(adcValue);
            tipPresent = adcValue<(ADC_MAXIMUM*0.9);
            break;

         default:
            // Can occur briefly when changing tools
            break;
      }
   };

   /**
    * Set duty cycle for use with fixed power mode
    *
    * @param dutyCycle
    */
   virtual void setDutyCycle(unsigned dutyCycle) {
      controller.setOutput(dutyCycle);
   }
   /**
    * Print single line report on control situation
    *
    * @param doHeading True to print a header first
    */
   virtual void report(bool doHeading) const {
      using namespace USBDM;

      if (doHeading) {
         console.write("Time,");
         controller.reportHeading(ch);
      }

      // Time-stamp
      console.setFloatFormat(2, Padding_LeadingSpaces, 3);
      console.write(controller.getElapsedTime());
      controller.report();

      console.write(",").write(getInstantTemperature());
      console.writeln();
      console.resetFormat();
   }
};

#endif /* T12_H_ */
