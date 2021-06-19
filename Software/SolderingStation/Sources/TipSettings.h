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

   enum Calib {
      Calib_250      = 0,  ///< Calibration value for 250 C
      Calib_350      = 1,  ///< Calibration value for 350 C
      Calib_400      = 2,  ///< Calibration value for 450 C
      Calib_Number   = 3,  ///< Number of calibration values
   };

   /// Index into tipNames table
   using TipNameIndex = uint8_t;

   /// Indicates unused entry in calibration table
   /// Corresponds to default flash value (0xFFFF)
   static constexpr TipNameIndex FREE_ENTRY = (TipNameIndex)-1;

   /// Indicates tip entry has been calibrated
   static constexpr uint16_t CALIBRATED = 1<<0;

private:
   /// Calibration values at three temperatures
   USBDM::NonvolatileArray<float, Calib_Number>thermocoupleVoltage;

   /// Calibration values at three temperatures
   USBDM::NonvolatileArray<float, Calib_Number>coldJunctionTemperature;

   /// Index into tip name table for this entry
   USBDM::Nonvolatile<TipNameIndex>    tipNameIndex;

   /// Flags for this entry
   USBDM::Nonvolatile<uint16_t>     properties;

   static constexpr unsigned NUMBER_OF_TIPS = 67;
   static const char *const tipNames[NUMBER_OF_TIPS];

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
    * Set values
    *
    * @param TipIndex  Tool index for this setting
    * @param t250       Calibration value for 250C
    * @param t350       Calibration value for 350C
    * @param t450       Calibration value for 450C
    */
   void initialise(TipNameIndex tipNameIndex) {
      this->tipNameIndex = tipNameIndex;
      this->properties   = 0;
      this->thermocoupleVoltage.set(Calib_250, 5.4);
      this->thermocoupleVoltage.set(Calib_350, 8.9);
      this->thermocoupleVoltage.set(Calib_400, 10);
      this->coldJunctionTemperature.set(Calib_250, 25);
      this->coldJunctionTemperature.set(Calib_350, 25);
      this->coldJunctionTemperature.set(Calib_400, 25);
   }

   /**
    * Set values from measured values
    *
    * @param other Dummy Tipsettings containing measurements
    */
   void setPoints(TipSettings &other) {
      properties = properties | CALIBRATED;
      thermocoupleVoltage     = other.thermocoupleVoltage;
      coldJunctionTemperature = other.coldJunctionTemperature;
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
      return thermocoupleVoltage[index];
   }

   /**
    * Get cold reference temperature
    *
    * @param index
    *
    * @return cold reference temperature
    */
   float coldJunction(Calib index) const {
      return coldJunctionTemperature[index];
   }

   /**
    * Indicates if the settings value have been generated from a calibration sequence.
    *
    * @return True  - Values have been calibrated.
    * @return False - Values are defaults.
    */
   bool isCalibrated() const {
      return (properties & CALIBRATED) != 0;
   }
};

#endif /* SOURCES_TIPSETTINGS_H_ */
