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

   /// Maximum value for duty-cycle
   unsigned upperLimit;

   /// Resolution (denominator of duty-cycle)
   const unsigned resolution;

   /// Count of cycles
   unsigned count;

   /// Duty-cycle (numerator)
   unsigned dutyCycle;

   /// Indicates if drive is on in the current cycle
   bool     driveOn;

   /// Enables output i.e. isOn() always returns false if false
   bool     enabled = false;

   /**
    * Update driveOn based on count
    */
   void check() {
      if (count >= resolution) {
         driveOn = true;
         count -= resolution;
      }
      else {
         driveOn = false;
      }
   }

public:
   /**
    * Constructor
    *
    * @param resolution Resolution (denominator of duty-cycle)
    */
   DutyCycleCounter(unsigned resolution) :
      upperLimit(resolution), resolution(resolution), count(0), dutyCycle(0), driveOn(false) {
   }

   /**
    * Disable output i.e. isOn() always return false
    */
   void disable() {
      enabled = false;
   }

   /**
    * Enable output i.e. isOn() returns value time dependent on PWM state
    */
   void enable() {
      enabled = true;
   }

   /**
    * Sets the upper limit for the duty cycle.
    *
    * @param upperLImit Upper limit for setDutyCycle()
    */
   void setUpperLimit(unsigned upperLImit) {
      this->upperLimit = upperLImit;
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
      if (dutyCycle>upperLimit) {
         dutyCycle = upperLimit;
      }
      this->dutyCycle = dutyCycle;
      check();
   }

   /**
    * Called to advance to the next interval
    */
   void advance() {
      count += dutyCycle;
      check();
   }

   /**
    * Is the drive to be on in the current interval
    *
    * @return True  => on
    * @return False => off
    */
   bool isOn() const {
      return enabled && driveOn;
   }
};

#endif /* SOURCES_DUTYCYCLECOUNTER_H_ */
