/**
 * @file    Pid.h
 * @brief   PID Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#ifndef PROJECT_HEADERS_PID_H_
#define PROJECT_HEADERS_PID_H_

#include "error.h"
#include "NonvolatileSettings.h"

/**
 * PID Controller
 */
class Pid {

public:
   const PidSettings &nvSettings;      //!< Proportional, Integral and Derivative tuning parameters

private:
   double   interval       = 10*USBDM::ms;     //!< Interval for sampling
   float    outMin         = 0.0;              //!< Minimum limit for output
   float    outMax         = 100.0;            //!< Maximum limit for output

   bool     enabled        = false;    //!< Enable for controller

   double   lastInput      = 0.0;      //!< Last input sample
   double   currentInput   = 0.0;      //!< Current input sample
   double   currentOutput  = 0.0;      //!< Current output
   double   currentError   = 0.0;      //!< Current error calculation

   double   integral       = 0.0;      //!< Integral accumulation term = sum(Ki * error(i)) * interval
   double   differential   = 0.0;      //!< Differential term          = Kd/interval * (S(i)-S(i-1))
   double   proportional   = 0.0;      //!< Proportional term          = Kp * error(i)

   unsigned tickCount      = 0;        //!< Time in ticks since last enabled

private:

   /**
    * Get proportional control factor
    *
    * @return factor as double
    */
   double getKp() {
      return  nvSettings.kp;
   }
   /**
    * Get integral control factor
    * This is scaled for internal use
    *
    * @return factor as double
    */
   double getKi() {
      return  nvSettings.ki*interval;
   }
   /**
    * Get differential control factor
    * This is scaled for internal use
    *
    * @return factor as double
    */
   double getKd() {
      return  nvSettings.kd/interval;
   }

public:
   /**
    * Constructor
    *
    * @param[in] Kp          Initial proportional constant
    * @param[in] Ki          Initial integral constant
    * @param[in] Kd          Initial differential constant
    * @param[in] outMin      Minimum value of output variable
    * @param[in] outMax      Maximum value of output variable
    */
   Pid(PidSettings &pidSettings) :
      nvSettings(pidSettings), enabled(false) {
   }

   /**
   * Destructor
   */
   virtual ~Pid() {
   }

   /**
    * Enable controller\n
    *
    * @note: Controller is re-initialised when enabled.
    * @note: Output is left unchanged when disabled.
    *
    * @param[in] enable True to enable
    */
   void enable(bool enable = true) {
      if (enable) {
         if (!enabled) {
            // Just enabled
            integral     = currentOutput;
            tickCount    = 0;
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

      integral += (getKi() * currentError);

      // Limit integral term
      if(integral > nvSettings.iLimit) {
         integral = nvSettings.iLimit;
      }
      else if(integral < -nvSettings.iLimit) {
         integral = -nvSettings.iLimit;
      }

      differential = getKd() * (currentInput - lastInput);

      proportional = getKp() * currentError;

      currentOutput = proportional + integral - differential;

      if(currentOutput > outMax) {
         currentOutput = outMax;
      }
      else if(currentOutput < outMin) {
         currentOutput = outMin;
      }
      // Update output
      return currentOutput;
   }

   void report() {
      USBDM::console.setFloatFormat(2, USBDM::Padding_LeadingSpaces, 5);
      USBDM::console.write(",").write(currentError).write(",").write(proportional).write(",").write(integral).write(",").writeln(differential);
      USBDM::console.resetFormat();
   }
};

#endif // PROJECT_HEADERS_PID_H_
