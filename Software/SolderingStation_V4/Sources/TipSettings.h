/*
 * TipSettings.h
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_TIPSETTINGS_H_
#define SOURCES_TIPSETTINGS_H_

#include "flash.h"

enum IronType : uint8_t {
   IronType_Unknown,
   IronType_Weller,
   IronType_T12,
};

class InitialTipInfo {
public:
   const char    *name;
   const IronType type;

   constexpr InitialTipInfo(const char *name, IronType type) : name(name), type(type) {}
};

enum CalibrationIndex {
   CalibrationIndex_250      = 0,  ///< Calibration value for 250 C
   CalibrationIndex_325      = 1,  ///< Calibration value for 325 C
   CalibrationIndex_400      = 2,  ///< Calibration value for 400 C
   CalibrationIndex_Number   = 3,  ///< Number of calibration values
};


inline CalibrationIndex operator++(CalibrationIndex &ci) {
   ci = CalibrationIndex(static_cast<unsigned>(ci) + 1);
   return ci;
}
/**
 * Nonvolatile data describing a tip
 * Unnecessary changes to data should be avoided to reduce EEPROM wear
 */
class TipSettings {

public:
   /**
    * Get the temperature for calibration at the given calibration point
    *
    * @param index Which calibration point is needed
    */
   static unsigned getCalibrationTemperature(CalibrationIndex index) {
      /// Temperatures for the three calibration points
      static const unsigned calibrationTemperatures[3] = {250, 325, 400};

      return calibrationTemperatures[index];
   }

   /// Number of tip settings available
   static constexpr unsigned NUM_TIP_SETTINGS = 20;

   /// Total number of tips available for selection
   static constexpr unsigned NUMBER_OF_TIPS   = 70;

   /// Names of all tips
   static const InitialTipInfo initialTipInfo[NUMBER_OF_TIPS];

   /// Index into tipNames table
   using TipNameIndex = uint8_t;

   /// Indicates unused entry in calibration table
   /// Corresponds to default flash value (0xFFFF)
   static constexpr TipNameIndex FREE_ENTRY    = (TipNameIndex)-1;

   /// Default tip (name) to use
   static constexpr TipNameIndex DEFAULT_ENTRY = (TipNameIndex)0;

   /// Indicates thermocouple and coldJunction values have been calibrated
   static constexpr uint16_t TEMP_CALIBRATED = 1<<0;

   /// Indicates tip PID values have been calibrated
   static constexpr uint16_t PID_CALIBRATED = 1<<1;

   /// Scale factor for storing non-volatile float values as 16-bit integer
   static constexpr int   FLOAT_SCALE_FACTOR   = 1000;

   /// Scale factor for storing non-volatile float values as A 16-bit integer
   static constexpr float FLOAT_SCALE_FACTOR_F = FLOAT_SCALE_FACTOR;

   /// Scale factor for storing non-volatile temperature values as a 16-bit integer
   static constexpr int TEMP_SCALE_FACTOR = 10;

   /// Scale factor for storing non-volatile temperature values as a 16-bit integer
   static constexpr float TEMP_SCALE_FACTOR_F = TEMP_SCALE_FACTOR;

private:
   /// First calibration value for each calibration point
   USBDM::NonvolatileArray<uint16_t, CalibrationIndex_Number>calibrationMeasurementValue;

   /// Second calibration value for each calibration point
   USBDM::NonvolatileArray<uint16_t, CalibrationIndex_Number>calibrationTemperatureValue;

   /// PID parameter - proportional constant
   USBDM::Nonvolatile<uint16_t> kp;

   /// PID parameter - integral constant
   USBDM::Nonvolatile<uint16_t> ki;

   /// PID parameter - differential constant
   USBDM::Nonvolatile<uint16_t> kd;

   /// PID parameter - limit on integral accumulation
   USBDM::Nonvolatile<uint16_t> iLimit;

   /// Flags for this entry
   USBDM::Nonvolatile<uint16_t>     flags;

   /// Index into tip name table for this entry
   USBDM::Nonvolatile<TipNameIndex>    tipNameIndex;

   TipSettings(const TipSettings &other) = delete;
   TipSettings(TipSettings &&other) = delete;
   TipSettings& operator=(TipSettings &&other) = delete;

public:
   TipSettings() {}

   ~TipSettings() {}

   TipSettings& operator=(const TipSettings &other) {
      this->kp = other.kp;
      this->ki = other.ki;
      this->kd = other.kd;
      this->flags = other.flags;
      this->tipNameIndex = other.tipNameIndex;
      for (CalibrationIndex index=CalibrationIndex_250; index<=CalibrationIndex_400; ++index) {
         calibrationMeasurementValue[index] = other.calibrationMeasurementValue[index];
         calibrationTemperatureValue[index] = other.calibrationTemperatureValue[index];
      }
      return *this;
   }

   /**
    * Get TipNameIndex for given tip name
    *
    * @param name  Tip name
    *
    * @return TipNameIndex (index into tip name table)
    */
   static TipNameIndex getTipNameIndex(const char *name);

   /**
    * Get tip name for given TipNameIndex
    *
    * @param index Index into name table
    *
    * @return Pointer to static string
    */
   static const char *getTipName(TipNameIndex index);

   /**
    * Indicates if this entry is free
    *
    * @return
    */
   bool isFree() const {
      return tipNameIndex == FREE_ENTRY;
   }

   /**
    * Frees this entry for re-use
    */
   void freeEntry() {
      tipNameIndex = FREE_ENTRY;
   }

   /**
    * Load default calibration values for given tip
    *
    * @param TipNameIndex  Tip name index for this setting
    */
   void loadDefaultCalibration(TipNameIndex tipNameIndex = DEFAULT_ENTRY);

   /**
    * Get PID Kp value
    *
    * @return Kp value
    */
   float getKp() const {
      return kp/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID Ki value
    *
    * @return Ki value
    */
   float getKi() const {
      return ki/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID Kd value
    *
    * @return Kd value
    */
   float getKd() const {
      return kd/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID iLimit value
    *
    * @return I limit value
    */
   float getILimit() const {
      return iLimit/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID Kp value
    *
    * @return Scaled Kp value (internal format)
    */
   float getRawKp() const {
      return kp;
   }

   /**
    * Get PID Ki value
    *
    * @return Scaled Ki value (internal format)
    */
   float getRawKi() const {
      return ki;
   }

   /**
    * Get PID Kd value
    *
    * @return Scaled Kd value (internal format)
    */
   float getRawKd() const {
      return kd;
   }

   /**
    * Get PID iLimit value
    *
    * @return Scaled I limit value (internal format)
    */
   float getRawILimit() const {
      return iLimit;
   }

   /**
    * Set modified raw PID control values (custom)
    *
    * @param kp      Scaled Kp value (internal format)
    * @param ki      Scaled Ki value (internal format)
    * @param kd      Scaled Ki value (internal format)
    * @param iLimit  Scaled ILimit value (internal format)
    */
   void setRawPidControlValues(int kp, int ki, int kd, int iLimit) {
      flags = flags | PID_CALIBRATED;
      this->kp     = kp;
      this->ki     = ki;
      this->kd     = kd;
      this->iLimit = iLimit;
   }

   /**
    * Set values from measured values
    *
    * @param other Dummy Tips-ettings containing measurements
    */
   void setThermisterCalibration(TipSettings &other) {
      flags = flags | TEMP_CALIBRATED;
      calibrationMeasurementValue = other.calibrationMeasurementValue;
      calibrationTemperatureValue = other.calibrationTemperatureValue;
   }

   /**
    * Set values from measured values
    *
    * @param other Dummy Tip-settings containing measurements
    */
   void setPidControlValues(TipSettings &other) {
      flags  = flags | PID_CALIBRATED;
      kp     = other.kp;
      ki     = other.ki;
      kd     = other.kd;
      iLimit = other.iLimit;
   }

   /**
    * Set initial PID control values (non-custom)
    *
    * @param kp
    * @param ki
    * @param kd
    * @param iLimit
    */
   void setInitialPidControlValues(float kp, float ki, float kd, float iLimit) {
      this->kp     = round(kp * FLOAT_SCALE_FACTOR);
      this->ki     = round(ki * FLOAT_SCALE_FACTOR);
      this->kd     = round(kd * FLOAT_SCALE_FACTOR);
      this->iLimit = round(iLimit * FLOAT_SCALE_FACTOR);
   }

   /**
    * Set a temperature calibration point
    *
    * @param calibrationIndex Index for the point
    * @param temperature      Temperature measurement value
    * @param measurement      Measurement value e.g. thermocouple voltage or PTC resistance
    */
   void setCalibrationPoint(CalibrationIndex calibrationIndex, float temperature, float measurement) {
      this->calibrationTemperatureValue.set(calibrationIndex, round(temperature*TEMP_SCALE_FACTOR));
      this->calibrationMeasurementValue.set(calibrationIndex, round(measurement*FLOAT_SCALE_FACTOR));
   }

   /**
    * Get calibration measurement voltage
    *
    * @param index
    *
    * @return Measurement value e.g. thermocouple voltage or PTC resistance
    */
   float getCalibrationMeasurementValue(CalibrationIndex index) const {
      return calibrationMeasurementValue[index]/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get calibration measurement temperature
    *
    * @param index
    *
    * @return Measurement temperature
    */
   float getCalibrationTempValue(CalibrationIndex index) const {
      return calibrationTemperatureValue[index]/TEMP_SCALE_FACTOR_F;
   }

   /**
    * Get TipNameIndex which this entry is associated with
    *
    * @return TipNameIndex
    */
   TipNameIndex getTipNameIndex() const {
      return tipNameIndex;
   }

   /**
    * Set TipNameIndex which this entry is associated with
    *
    * @param tipNameIndex
    */
   void setTipNameIndex(TipNameIndex tipNameIndex) {
      this->tipNameIndex = tipNameIndex;
   }

   /**
    * Get name of Tool which this entry is associated with
    *
    * @return Pointer to tip name as static object
    */
   const char *getTipName() const {
      if (tipNameIndex == FREE_ENTRY) {
         return "Unallocated";
      }
      return initialTipInfo[tipNameIndex].name;
   }

   /**
    * Get name of Tool which this entry is associated with
    *
    * @return Pointer to tip name as static object
    */
   IronType getIronType() const {
      if (tipNameIndex == FREE_ENTRY) {
         return IronType_T12;
      }
      return initialTipInfo[tipNameIndex].type;
   }

   /**
    * Indicates if the temperature values have been generated from a calibration sequence.
    *
    * @return True  - Values have been calibrated.
    * @return False - Values are defaults.
    */
   bool isTemperatureCalibrated() const {
      return (flags & TEMP_CALIBRATED) != 0;
   }

   /**
    * Indicates if the PID values have been generated from a calibration sequence.
    *
    * @return True  - Values have been calibrated.
    * @return False - Values are defaults.
    */
   bool isPidCalibrated() const {
      return (flags & PID_CALIBRATED) != 0;
   }
};

#endif /* SOURCES_TIPSETTINGS_H_ */
