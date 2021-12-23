/*
 * TipSettings.h
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_TIPSETTINGS_H_
#define SOURCES_TIPSETTINGS_H_

#include "formatted_io.h"
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

   /// Number of tip settings available on on-volatile storage
   static constexpr unsigned NUM_TIP_SETTINGS = 20;

   /// Type for index into initialTipInfo table in ROM
   enum TipNameIndex : uint8_t {
      /// Indicates a failed result (Invalid tip - dummy entry)
      NO_TIP = 0,

      /// Total number of tips available for selection (excludes NO_TIP)
      NUMBER_OF_VALID_TIPS   = 70,

      /// Indicates unused entry in calibration table
      /// Corresponds to default flash value (0xFFFF)
      FREE_ENTRY    = (TipNameIndex)-1,
   };

   /// First real tip in initialTipInfo array
   static const enum TipNameIndex FIRST_VALID_TIP = (TipNameIndex)1;

   /// Last real tip in initialTipInfo array
   static const enum TipNameIndex LAST_VALID_TIP = NUMBER_OF_VALID_TIPS;

private:

   /// First tip in initialTipInfo array
   static const enum TipNameIndex FIRST_TIP = (TipNameIndex)1;

   /// Size of initialTipInfo array
   static const enum TipNameIndex SIZE_OF_TIP_ARRAY = (TipNameIndex)(LAST_VALID_TIP + FIRST_VALID_TIP);

public:
   /**
    * Convert an integer to TipNameIndex
    *
    * @param index Index to convert
    *
    * @return Resulting TipNameIndex
    */
   static TipNameIndex tipNameIndex(int index) {
      usbdm_assert((static_cast<unsigned>(index)<=LAST_VALID_TIP), "Range error on conversion to TipNameIndex");
      return TipNameIndex(index);
   }

   static TipNameIndex &inc(TipNameIndex &index) {
      index = static_cast<TipNameIndex>(index+1);
      return index;
   }
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

   /// Names of all tips
   static const InitialTipInfo initialTipInfo[SIZE_OF_TIP_ARRAY];

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
   USBDM::NonvolatileArray<uint16_t, CalibrationIndex_Number>nvCalibrationMeasurementValue;

   /// Second calibration value for each calibration point
   USBDM::NonvolatileArray<uint16_t, CalibrationIndex_Number>nvCalibrationTemperatureValue;

   /// PID parameter - proportional constant
   USBDM::Nonvolatile<uint16_t> nvKp;

   /// PID parameter - integral constant
   USBDM::Nonvolatile<uint16_t> nvKi;

   /// PID parameter - differential constant
   USBDM::Nonvolatile<uint16_t> nvKd;

   /// PID parameter - limit on integral accumulation
   USBDM::Nonvolatile<uint16_t> nvILimit;

   /// Flags for this entry
   USBDM::Nonvolatile<uint16_t>     nvFlags;

   /// Index into tip name table for this entry
   USBDM::Nonvolatile<TipNameIndex>    nvTipNameIndex;

//   TipSettings(TipSettings &&other) = delete;
//   TipSettings& operator=(TipSettings &&other) = delete;

public:
   TipSettings() {}

   ~TipSettings() {}

   TipSettings& operator=(const TipSettings &other) = default;

   /**
    * The is intended to allow creating a RAM-based copy of a non-volatile settings object.
    *
    * It does not correctly copy to flash!
    *
    * @param other
    */
   TipSettings(const TipSettings &other) = default;

   TipSettings(TipNameIndex tipNameIndex) {
      this->setTipNameIndex(tipNameIndex);
   }

   bool isNoTip() {
      return nvTipNameIndex == NO_TIP;
   }

   /**
    * Report settings object
    *
    * @param io Where to write report
    */
   void report(USBDM::FormattedIO &io);

   /**
    * Get TipNameIndex for given tip name
    *
    * @param tipName  Tip name
    *
    * @return TipNameIndex (index into tip name table)
    */
   static TipNameIndex getTipNameIndex(const char *tipName);

   /**
    * Get tip name for given TipNameIndex
    *
    * @param index Index into name table
    *
    * @return Pointer to static string
    */
   static const char *getTipName(TipNameIndex index);

   static const char *getIronTypeName(IronType ironType);

   /**
    * Indicates if this entry is free
    *
    * @return
    */
   bool isFree() const {
      return nvTipNameIndex == FREE_ENTRY;
   }

   /**
    * Frees this entry for re-use
    */
   void freeEntry() {
      nvTipNameIndex = FREE_ENTRY;
   }

   /**
    * Load default calibration values for given tip
    *
    * @param tipNameIndex  Tip name index for this setting
    */
   void loadDefaultCalibration(TipNameIndex tipNameIndex);

   /**
    * Get PID Kp value
    *
    * @return Kp value
    */
   float getKp() const {
      return nvKp/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID Ki value
    *
    * @return Ki value
    */
   float getKi() const {
      return nvKi/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID Kd value
    *
    * @return Kd value
    */
   float getKd() const {
      return nvKd/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID iLimit value
    *
    * @return I limit value
    */
   float getILimit() const {
      return nvILimit/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get PID Kp value
    *
    * @return Scaled Kp value (internal format)
    */
   float getRawKp() const {
      return nvKp;
   }

   /**
    * Get PID Ki value
    *
    * @return Scaled Ki value (internal format)
    */
   float getRawKi() const {
      return nvKi;
   }

   /**
    * Get PID Kd value
    *
    * @return Scaled Kd value (internal format)
    */
   float getRawKd() const {
      return nvKd;
   }

   /**
    * Get PID iLimit value
    *
    * @return Scaled I limit value (internal format)
    */
   float getRawILimit() const {
      return nvILimit;
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
      nvFlags = nvFlags | PID_CALIBRATED;
      nvKp     = kp;
      nvKi     = ki;
      nvKd     = kd;
      nvILimit = iLimit;
   }

   /**
    * Set values from measured values
    *
    * @param other Dummy Tips-settings containing measurements
    */
   void setThermisterCalibration(TipSettings &other) {
      nvFlags = nvFlags | TEMP_CALIBRATED;
      nvCalibrationMeasurementValue = other.nvCalibrationMeasurementValue;
      nvCalibrationTemperatureValue = other.nvCalibrationTemperatureValue;
   }

   /**
    * Set values from measured values
    *
    * @param other Dummy Tip-settings containing measurements
    */
   void setPidControlValues(TipSettings &other) {
      nvFlags  = nvFlags | PID_CALIBRATED;
      nvKp     = other.nvKp;
      nvKi     = other.nvKi;
      nvKd     = other.nvKd;
      nvILimit = other.nvILimit;
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
      nvKp     = round(kp * FLOAT_SCALE_FACTOR);
      nvKi     = round(ki * FLOAT_SCALE_FACTOR);
      nvKd     = round(kd * FLOAT_SCALE_FACTOR);
      nvILimit = round(iLimit * FLOAT_SCALE_FACTOR);
   }

   /**
    * Set a temperature calibration point
    *
    * @param calibrationIndex Index for the point
    * @param temperature      Temperature measurement value
    * @param measurement      Measurement value e.g. thermocouple voltage or PTC resistance
    */
   void setCalibrationPoint(CalibrationIndex calibrationIndex, float temperature, float measurement) {
      this->nvCalibrationTemperatureValue.set(calibrationIndex, round(temperature*TEMP_SCALE_FACTOR));
      this->nvCalibrationMeasurementValue.set(calibrationIndex, round(measurement*FLOAT_SCALE_FACTOR));
   }

   /**
    * Get calibration measurement voltage
    *
    * @param index
    *
    * @return Measurement value e.g. thermocouple voltage or PTC resistance
    */
   float getCalibrationMeasurementValue(CalibrationIndex index) const {
      return nvCalibrationMeasurementValue[index]/FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get calibration measurement temperature
    *
    * @param index
    *
    * @return Measurement temperature
    */
   float getCalibrationTempValue(CalibrationIndex index) const {
      return nvCalibrationTemperatureValue[index]/TEMP_SCALE_FACTOR_F;
   }

   /**
    * Get TipNameIndex which this entry is associated with
    *
    * @return TipNameIndex
    */
   TipNameIndex getTipNameIndex() const {
      return nvTipNameIndex;
   }

   /**
    * Set TipNameIndex which this entry is associated with
    *
    * @param tipNameIndex
    */
   void setTipNameIndex(TipNameIndex tipNameIndex) {
      nvTipNameIndex = tipNameIndex;
   }

   /**
    * Get TipNameIndex for 1st entry applicable to this iron type
    *
    * @param ironType
    *
    * @return Index of entry found or NO_TIP if not found
    */
   static TipNameIndex getDefaultTipForIron(IronType ironType) {
      for (TipNameIndex index=FIRST_VALID_TIP; index<=LAST_VALID_TIP; inc(index)) {
         if (initialTipInfo[index].type == ironType) {
            return index;
         }
      }
      return NO_TIP;
   }

   /**
    * Get name of Tool which this entry is associated with
    *
    * @return Pointer to tip name as static object
    */
   const char *getTipName() const {
      if (nvTipNameIndex == FREE_ENTRY) {
         return initialTipInfo[NO_TIP].name;
      }
      return initialTipInfo[nvTipNameIndex].name;
   }

   /**
    * Get name of Tool which this entry is associated with
    *
    * @return Pointer to tip name as static object
    */
   IronType getIronType() const {
      if (nvTipNameIndex == FREE_ENTRY) {
         return IronType_Unknown;
      }
      return initialTipInfo[nvTipNameIndex].type;
   }

   /**
    * Indicates if the temperature values have been generated from a calibration sequence.
    *
    * @return True  - Values have been calibrated.
    * @return False - Values are defaults.
    */
   bool isTemperatureCalibrated() const {
      return (nvFlags & TEMP_CALIBRATED) != 0;
   }

   /**
    * Indicates if the PID values have been generated from a calibration sequence.
    *
    * @return True  - Values have been calibrated.
    * @return False - Values are defaults.
    */
   bool isPidCalibrated() const {
      return (nvFlags & PID_CALIBRATED) != 0;
   }
};

#endif /* SOURCES_TIPSETTINGS_H_ */
