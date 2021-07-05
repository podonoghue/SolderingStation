/*
 * TipSettings.h
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_TIPSETTINGS_H_
#define SOURCES_TIPSETTINGS_H_

#include "flash.h"

class TipSettings {
   friend class Menus;

public:
   /// Number of tip settings available
   static constexpr unsigned NUM_TIP_SETTINGS = 20;

   /// Total number of tips available for selection
   static constexpr unsigned NUMBER_OF_TIPS   = 67;

   /// Names of all tips
   static const char *const tipNames[NUMBER_OF_TIPS];

   enum Calib {
      Calib_250      = 0,  ///< Calibration value for 250 C
      Calib_350      = 1,  ///< Calibration value for 350 C
      Calib_400      = 2,  ///< Calibration value for 400 C
      Calib_Number   = 3,  ///< Number of calibration values
   };

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

   /// Scale factor for storing non-volatile float values as 16-bit number
   static constexpr int   FLOAT_SCALE_FACTOR   = 1000;

   /// Scale factor for storing non-volatile float values as 16-bit number
   static constexpr float FLOAT_SCALE_FACTOR_F = FLOAT_SCALE_FACTOR;

private:
   /// Thermocouple voltages at three temperatures
   USBDM::NonvolatileArray<uint16_t, Calib_Number>thermocoupleVoltage;

   /// Cold junction temperature values at three temperatures
   USBDM::NonvolatileArray<uint16_t, Calib_Number>coldJunctionTemperature;

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
   TipSettings& operator=(const TipSettings &other) = delete;
   TipSettings& operator=(TipSettings &&other) = delete;

public:
   TipSettings() {}

   ~TipSettings() {}

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
    * Set set default calibration values
    *
    * @param TipNameIndex  Tip name index for this setting
    */
   void setDefaultCalibration(TipNameIndex tipNameIndex = DEFAULT_ENTRY) {
      this->tipNameIndex = tipNameIndex;
      this->flags   = 0;

      this->thermocoupleVoltage.set(Calib_250, 5.4*FLOAT_SCALE_FACTOR);
      this->thermocoupleVoltage.set(Calib_350, 8.9*FLOAT_SCALE_FACTOR);
      this->thermocoupleVoltage.set(Calib_400, 10*FLOAT_SCALE_FACTOR);

      this->coldJunctionTemperature.set(Calib_250, 25*FLOAT_SCALE_FACTOR);
      this->coldJunctionTemperature.set(Calib_350, 25*FLOAT_SCALE_FACTOR);
      this->coldJunctionTemperature.set(Calib_400, 25*FLOAT_SCALE_FACTOR);

      this->kp       =  5.0*FLOAT_SCALE_FACTOR;
      this->ki       =  0.5*FLOAT_SCALE_FACTOR;
      this->kd       =  0.0*FLOAT_SCALE_FACTOR;
      this->iLimit   = 40.0*FLOAT_SCALE_FACTOR;
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
    * Set values from measured values
    *
    * @param other Dummy Tipsettings containing measurements
    */
   void setThermisterCalibration(TipSettings &other) {
      flags = flags | TEMP_CALIBRATED;
      thermocoupleVoltage     = other.thermocoupleVoltage;
      coldJunctionTemperature = other.coldJunctionTemperature;
   }

   /**
    * Set values from measured values
    *
    * @param other Dummy Tipsettings containing measurements
    */
   void setPidControlValues(TipSettings &other) {
      flags  = flags | PID_CALIBRATED;
      kp     = other.kp;
      ki     = other.ki;
      kd     = other.kd;
      iLimit = other.iLimit;
   }

   /**
    * Get TipIndex which this entry is associated with
    *
    * @return TipIndex
    */
   TipNameIndex getTipNameIndex() const {
      return tipNameIndex;
   }

   /**
    * Get name of Tool which this entry is associated with
    *
    * @return TipIndex
    */
   const char *getTipName() const {
      if (tipNameIndex == FREE_ENTRY) {
         return "Unallocated";
      }
      return tipNames[tipNameIndex];
   }

   /**
    * Get thermocouple voltage
    *
    * @param index
    *
    * @return thermocouple voltage
    */
   float thermocouple(Calib index) const {
      return thermocoupleVoltage[index]*FLOAT_SCALE_FACTOR_F;
   }

   /**
    * Get cold reference temperature
    *
    * @param index
    *
    * @return cold reference temperature
    */
   float coldJunction(Calib index) const {
      return coldJunctionTemperature[index]*FLOAT_SCALE_FACTOR_F;
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
