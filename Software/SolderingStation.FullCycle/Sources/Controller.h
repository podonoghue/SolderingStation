/*
 * ControllerX.h
 *
 *  Created on: 2 Jul. 2021
 *      Author: peter
 */

#ifndef SOURCES_CONTROLLER_H_
#define SOURCES_CONTROLLER_H_

#include "TipSettings.h"

class Channel;

class Controller {

protected:
   /// Enable for controller
   bool        fEnabled        = false;

   /// Time in ticks since last enabled
   unsigned    fTickCount      = 0;

   /// Interval for sampling
   const float fInterval;

   /// Current input sample
   double      fCurrentInput   = 0.0;

   /// Current output
   double      fCurrentOutput  = 0.0;

   /// Current error calculation
   double      fCurrentError   = 0.0;

   /// Minimum limit for output
   const float fOutMin;

   /// Maximum limit for output
   const float fOutMax;

public:
   Controller(float interval, float outMin, float outMax) : fInterval(interval), fOutMin(outMin), fOutMax(outMax) {
   }

   virtual ~Controller() {}

   /**
    * Get number of seconds since last enabled
    *
    * @return Elapsed time
    */
   double getElapsedTime() {
      return (fTickCount*fInterval);
   }

   /**
    * Set output of controller
    * This is used when the controller is disabled
    *
    * @param newOutput New output value
    */
   void setOutput(double newOutput) {
      fCurrentOutput = newOutput;
   }

   /**
    * Get error of controller
    *
    * @return Last error calculation
    */
   double getError() {
      return fCurrentError;
   }

   /**
    * Indicates if the controller is enabled
    *
    * @return True => enabled
    */
   bool isEnabled() {
      return fEnabled;
   }

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
   virtual double newSample(float actualTemperature, float targetTemperature) = 0;

   /**
    * Set control parameters
    *
    * @param settings Parameter to use
    */
   virtual void setControlParameters(const TipSettings *settings) = 0;

   /**
    * Enable controller\n
    *
    * @note: Controller is re-initialised when enabled.
    * @note: Output is left unchanged when disabled.
    *
    * @param[in] enable True to enable
    */
   virtual void enable(bool enable = true) = 0;

   /**
    * Report current situation
    */
   virtual void report(Channel &ch) = 0;

   /**
    * Print heading for report()
    */
   virtual void reportHeading(Channel &ch) = 0;

};

#endif /* SOURCES_CONTROLLER_H_ */
