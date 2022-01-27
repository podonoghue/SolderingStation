/*
 * Controller.h
 *
 *  Created on: 2 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_CONTROLLER_H_
#define SOURCES_CONTROLLER_H_

#include "TipSettings.h"
#include "DutyCycleCounter.h"

class Channel;

/**
 * Base class for controllers
 */
class Controller : public DutyCycleCounter {

protected:
   /// Time in ticks since last enabled
   unsigned    fTickCount      = 0;

   /// Interval for sampling
   const USBDM::Seconds fInterval;

   /// Current input sample
   float       fCurrentInput   = 0.0;

   /// Current output
   float       fCurrentOutput  = 0.0;

   /// Current target temperature
   float       fCurrentTarget  = 0.0;

   /// Current error calculation
   float       fCurrentError   = 0.0;

   float       fDifferential   = 0.0;
   float       fProportional   = 0.0;

   /// Minimum limit for output
   const float fOutMin;

   /// Maximum limit for output
   const float fOutMax;


   /// Enables output i.e. isOn() always returns false if false
   bool     fEnabled;

public:
   Controller(USBDM::Seconds interval, float outMin, float outMax) :
      DutyCycleCounter(101),
      fInterval(interval), fOutMin(outMin), fOutMax(outMax), fEnabled(false) {
   }

   virtual ~Controller() {}

   /**
    * Indicates if the output is enabled
    *
    * @return True  - Enabled
    * @return False - Disabled
    */
   bool isEnabled() const {
      return fEnabled;
   }

   /**
    * Get number of seconds since last enabled
    *
    * @return Elapsed time
    */
   USBDM::Seconds getElapsedTime() const {
      return (fTickCount*fInterval);
   }

   /**
    * Set output of controller
    * This is used when the controller is disabled
    *
    * @param newOutput New output value
    */
   void setOutput(float newOutput) {
      fCurrentOutput = newOutput;
   }

   /**
    * Get error of controller
    *
    * @return Last error calculation
    */
   float getError() const {
      return fCurrentError;
   }

   /**
    * Set control parameters
    *
    * @param settings Parameter to use
    */
   virtual void setControlParameters(const TipSettings *settings) = 0;

   /**
    * Main calculation
    *
    * Should be executed at interval period
    *
    * Process new sample to produce new control output
    *
    * @param actualTemperature   Tip temperature in Celsius
    * @param targetTemperature   Target tip temperature in Celsius
    *
    * @return Control output
    */
   virtual float newSample(float actualTemperature, float targetTemperature) = 0;

   /**
    * Enable controller
    *
    * @note: Controller may be re-initialised when enabled.
    * @note: Output is left unchanged when disabled.
    *
    * @param[in] enable True to enable
    */
   virtual void enable(bool enable = true) = 0;

   /**
    * Report current situation
    */
   virtual void report() const = 0;

   /**
    * Print heading for report()
    */
   virtual void reportHeading(Channel &ch) const = 0;
};

class PidController;
class TakeBackHalfController;

//   using TempController = BangBangController;
   using TempController = PidController;
//   using TempController = TakeBackHalfController;

#endif /* SOURCES_CONTROLLER_H_ */
