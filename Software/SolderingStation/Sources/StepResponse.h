/**
 * @file    StepResponse.h
 * @brief   Bang-Bang Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#ifndef PROJECT_HEADERS_STEPRESPONSE_H_
#define PROJECT_HEADERS_STEPRESPONSE_H_

#include "error.h"
#include "NonvolatileSettings.h"

class Channel;

/**
 * Dummy Controller
 */
class StepResponse {

private:
   Channel &channel;                         ///< Associated channel

   double   interval       = 10*USBDM::ms;   ///< Interval for sampling
   float    outMin         = 0.0;            ///< Minimum limit for output
   float    outMax         = 100.0;          ///< Maximum limit for output

   bool     enabled        = false;          ///< Enable for controller

   double   lastInput      = 0.0;            ///< Last input sample
   double   currentInput   = 0.0;            ///< Current input sample
   double   currentOutput  = 0.0;            ///< Current output
   double   currentError   = 0.0;            ///< Current error calculation

   unsigned tickCount      = 0;              ///< Time in ticks since last enabled
   unsigned cycleCount     = 0;


public:
   /**
    * Constructor
    */
   StepResponse(Channel &channel) : channel(channel) { }

   /**
   * Destructor
   */
   virtual ~StepResponse() {
   }

   /**
    * Enable controller\n
    * Note: Controller is re-initialised when enabled
    *
    * @param[in] enable True to enable
    */
   void enable(bool enable = true) {
      if (enable) {
         if (!enabled) {
            // Just enabled
            tickCount    = 0;
            cycleCount = 0;
         }
      }
      enabled = enable;
   }

   /**
    * Set parameters being used
    *
    * @param[in] interval  Sample interval for controller in seconds
    * @param[in] min       Minimum value of output variable
    * @param[in] max       Maximum value of output variable
    */
   void setParameters(double newInterval, float min, float max) {
      interval = newInterval;
      outMin   = min;
      outMax   = max;
   }

   /**
    * Indicates if the controller is enabled
    *
    * @return True => enabled
    */
   bool isEnabled() {
      return enabled;
   }

   /**
    * Get number of ticks since last enabled
    *
    * @return Number of ticks
    */
   unsigned getTicks() {
      return tickCount;
   }

   /**
    * Get number of seconds since last enabled
    *
    * @return Elapsed time
    */
   double getElapsedTime() {
      return (tickCount*interval);
   }

   /**
    * Get input of controller
    *
    * @return Last input sample
    */
   double getInput() {
      return currentInput;
   }

   /**
    * Get output of controller
    *
    * @return Last output value
    */
   double getOutput() {
      return currentOutput;
   }

   /**
    * Set output of controller
    * This is used when the controller is disabled
    *
    * @param newOutput New output value
    */
   void setOutput(double newOutput) {
      currentOutput = newOutput;
   }

   /**
    * Get error of controller
    *
    * @return Last error calculation
    */
   double getError() {
      return currentError;
   }

   /**
    * Main PID calculation
    *
    * Should be executed at interval period
    *
    * @param setpoint   Desired operating point
    * @param sample     Current (sampled) operating point
    *
    * @return  Adjustment predicted
    */
   double newSample(double setpoint, double sample) {

      if(!enabled) {
         return currentOutput;
      }

      tickCount++;

      // Update input samples & error
      lastInput    = currentInput;
      currentInput = sample;
      currentError = setpoint - currentInput;

      static constexpr unsigned minValue = 1;
      static constexpr unsigned maxValue = 5;

      const unsigned INITIAL_INTERVAL = 50/interval;
      const unsigned STEP_INTERVAL    = 500/interval;

      if (tickCount<INITIAL_INTERVAL) {
         currentOutput = 0;
         cycleCount    = 0;
      }
      else {
         if ((tickCount % STEP_INTERVAL) == INITIAL_INTERVAL) {
            cycleCount++;
            currentOutput = (cycleCount&1)?maxValue:minValue;
         }
         if (cycleCount > 4) {
            currentOutput = 0;
            enable(false);
         }
      }

      // Update output
      return currentOutput;
   }

   void report() {
      USBDM::console.writeln();
   }
};

#endif // PROJECT_HEADERS_STEPRESPONSE_H_
