/*
 * Measurement.h
 *
 *  Created on: 14 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_MEASUREMENT_H_
#define SOURCES_MEASUREMENT_H_

#include "TipSettings.h"
#include "stringFormatter.h"

class Measurement {
protected:
   static constexpr unsigned BUF_SIZE = 40;

   /// Static buffer used for reporting calibration values.
   /// Static as used as return value.
   /// Note - hence usage is not re-entrant!!!
   static USBDM::StringFormatter_T<BUF_SIZE> &getStringFormatter() {
      static USBDM::StringFormatter_T<BUF_SIZE> fStringFormatter;
      return fStringFormatter;
   }

public:
   Measurement() {}

   virtual ~Measurement() {}

   /**
    * Get average tip temperature
    *
    * @return Tip temperature in Celsius
    */
   virtual float getTemperature() = 0;

   /**
    * Get tip temperature from last sample
    *
    * @return Tip temperature in Celsius
    */
   virtual float getInstantTemperature() = 0;

   /**
    * Get value for resistance of heating element (for power calculations)
    *
    * @return Resistance in ohms
    */
   virtual float getHeaterResistance() const = 0;

   /**
    * Set calibration data from current measurements
    *
    * @param[in]  calibrationIndex Index for the calibration
    * @param[out] tipSettings      Tip-settings to update with calibratin point
    */
   virtual void saveCalibrationPoint(CalibrationIndex calibrationIndex, TipSettings &tipSettings) = 0;

   /**
    * Get a report of the calibration values for the current operating point.
    * Used for reporting during temperature calibration.
    *
    * @return Point to static string
    *
    * @note Not re-entrant!
    */
   virtual const char *reportCalibrationValues() = 0;

   /**
    * Set calibration data
    *
    * @param[in] tipsettings Settings object with calibration data
    */
   virtual void setCalibrationValues(const TipSettings *tipsettings) = 0;

   /**
    * Get 'measurement' value for debug when calibrating
    *
    * @return
    */
   virtual float getMeasurement() const = 0;

   /**
    * Get the sequence of ADC measurements to do
    *
    * @return Sequence array terminated by MuxSelect_Complete sentinel
    */
   virtual MuxSelect const *getMeasurementSequence() = 0;

   /**
    * Get the sequence of ADC measurements to do
    *
    * @param[out] seq         Array of measurements to do
    * @param[in]  channelMask Mask indicating which channel
    *
    * @return Number of measurements added to seq[]
    */
   unsigned getMeasurementSequence(MuxSelect seq[], uint8_t channelMask) {
      MuxSelect const *newSequence = getMeasurementSequence();
      unsigned sequenceLength;
      for(sequenceLength=0; newSequence[sequenceLength] != MuxSelect_Complete; sequenceLength++) {
         // Add channel information
         seq[sequenceLength] = (MuxSelect)(newSequence[sequenceLength]|channelMask);
      }
      return sequenceLength;
   }

   /**
    * Process ADC measurement value
    *
    * @param[in] muxSelect  Indicates which measurement made
    * @param[in] adcValue   ADC value for measurement
    */
   virtual void processMeasurement(MuxSelect muxSelect, uint32_t adcValue) = 0;
};

class DummyMeasurement : public Measurement {
public:
   virtual float getHeaterResistance() const override { return 8.0; };
   virtual float getTemperature() override { return 1.0;};
   virtual float getInstantTemperature() override { return 1.0; };
   virtual void  saveCalibrationPoint(CalibrationIndex, TipSettings &) override {};
   virtual const char *reportCalibrationValues() override { return ""; };
   virtual float getMeasurement() const override { return 0.0; };
   virtual void setCalibrationValues(const TipSettings *) override { }
   virtual MuxSelect const *getMeasurementSequence() override {
      static const MuxSelect dummy = MuxSelect_Complete;
      return &dummy;
   };
   virtual void processMeasurement(MuxSelect, uint32_t) override {};
};

#endif /* SOURCES_MEASUREMENT_H_ */
