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

class Channel;

/**
 * PID Controller
 */
class Pid {

private:
   float    interval       = 10*USBDM::ms;     //!< Interval for sampling
   float    outMin         = 0.0;              //!< Minimum limit for output
   float    outMax         = 100.0;            //!< Maximum limit for output

   bool     enabled        = false;    //!< Enable for controller

   double   kp             = 0.0;
   double   ki             = 0.0;
   double   kd             = 0.0;
   double   iLimit         = 0.0;

   double   lastInput      = 0.0;      //!< Last input sample
   double   currentInput   = 0.0;      //!< Current input sample
   double   currentOutput  = 0.0;      //!< Current output
   double   currentError   = 0.0;      //!< Current error calculation

   double   integral       = 0.0;      //!< Integral accumulation term = sum(Ki * error(i)) * interval
   double   differential   = 0.0;      //!< Differential term          = Kd/interval * (S(i)-S(i-1))
   double   proportional   = 0.0;      //!< Proportional term          = Kp * error(i)

   unsigned tickCount      = 0;        //!< Time in ticks since last enabled

   /**
    * Get proportional control factor
    *
    * @return factor as double
    */
   double getKp() const { return kp; };

   /**
    * Get integral control factor
    * This is scaled for internal use
    *
    * @return factor as double
    */
   double getKi() const { return ki; };

   /**
    * Get differential control factor
    * This is scaled for internal use
    *
    * @return factor as double
    */
   double getKd() const {return kd; };

public:

   /**
    * Constructor
    *
    * @param channel Associated channel
    */
   Pid() {
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
   void setParameters(float newInterval, float min, float max);

   /**
    * Set control parameters
    *
    * @param kp
    * @param ki
    * @param kd
    * @param iLimit
    */
   void setControlParameters(float kp, float ki, float kd, float iLimit);

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
   double newSample(double setpoint, double sample);

   void report();
};

#endif // PROJECT_HEADERS_PID_H_
