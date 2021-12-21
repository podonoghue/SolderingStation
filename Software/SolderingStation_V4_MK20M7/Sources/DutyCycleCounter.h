/*
 * DutyCycleCounter.h
 *
 *  Created on: 7 Jul 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_DUTYCYCLECOUNTER_H_
#define SOURCES_DUTYCYCLECOUNTER_H_

/**
 * This class implements a variable period PWM
 */
class DutyCycleCounter {

protected:
   /// Maximum value for duty-cycle
   unsigned fUpperLimit;

   /// Resolution (denominator of duty-cycle)
   const unsigned fResolution;

   /// Count of cycles
   unsigned fCount;

   /// Duty-cycle (numerator)
   unsigned fDutyCycle;

   /// Indicates if drive is on in the current cycle
   bool     fDriveOn;

   /**
    * Update driveOn based on count
    */
   void check() {
      USBDM::CriticalSection cs;
      if (fCount >= fResolution) {
         fDriveOn = true;
         fCount -= fResolution;
      }
      else {
         fDriveOn = false;
      }
   }

public:
   /**
    * Constructor
    *
    * @param resolution Resolution (denominator of duty-cycle)
    */
   DutyCycleCounter(unsigned resolution) :
      fUpperLimit(resolution), fResolution(resolution), fCount(0), fDutyCycle(0), fDriveOn(false) {
   }

   /**
    * Sets the upper limit for the duty cycle.
    *
    * @param upperLimit Upper limit for setDutyCycle()
    */
   void setUpperLimit(unsigned upperLimit) {
      this->fUpperLimit = upperLimit;
   }

   /**
    * Set duty-cycle
    *
    * @param dutyCycle Duty-cycle as a fraction of resolution
    *
    * @note Value is clipped to setUpperLimit() value
    * @note The average duty-cycle is dutyCycle/resolution
    */
   void setDutyCycle(unsigned dutyCycle) {
      if (dutyCycle>fUpperLimit) {
         dutyCycle = fUpperLimit;
      }
      fDutyCycle = dutyCycle;
   }

   /***
    * Get currently set duty cycle
    *
    * @return Duty-cycle as a fraction of resolution
    *
    */
   unsigned getDutyCycle() const {
      return fDutyCycle;
   }

   /**
    * Advance the PWM sequence
    *
    * @param borrowCycle   Indicates if the next cycle must be off for measurement.
    */
   void advance() {
      fCount += fDutyCycle;
      check();
   }

   /**
    * Is the drive to be on in the current interval
    *
    * @return True  => on
    * @return False => off
    */
   bool isOn() const {
      return fDriveOn;
   }
};

#endif /* SOURCES_DUTYCYCLECOUNTER_H_ */
