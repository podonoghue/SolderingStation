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

/**
 * PID Controller
 */
class Pid {

private:
   const double interval;     //!< Interval for sampling
   const float outMin;        //!< Minimum limit for output
   const float outMax;        //!< Maximum limit for output

   double kp;                 //!< Proportional Tuning Parameter
   double ki;                 //!< Integral Tuning Parameter
   double kd;                 //!< Derivative Tuning Parameter

   bool   enabled;            //!< Enable for controller

   double integral;           //!< Integral accumulation term

   double lastInput;          //!< Last input sample
   double currentInput;       //!< Current input sample
   double currentOutput;      //!< Current output
//   double setpoint;           //!< Set-point for controller
   double currentError;       //!< Current error calculation

   unsigned tickCount = 0;    //!< Time in ticks since last enabled

public:
   /**
    * Constructor
    *
    * @param[in] Kp          Initial proportional constant
    * @param[in] Ki          Initial integral constant
    * @param[in] Kd          Initial differential constant
    * @param[in] interval    Sample interval for controller
    * @param[in] outMin      Minimum value of output variable
    * @param[in] outMax      Maximum value of output variable
    */
   Pid(double Kp, double Ki, double Kd, double interval, float outMin, float outMax) :
      interval(interval), outMin(outMin), outMax(outMax), enabled(false) {
      setTunings(Kp, Ki, Kd);
   }

   /**
   * Destructor
   */
   virtual ~Pid() {
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
//            currentInput = inputFn();
            integral     = 0; //currentOutput;
            tickCount    = 0;
         }
      }
      enabled = enable;
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
    * Change controller tuning
    *
    * @param[in] Kp Proportional constant
    * @param[in] Ki Integral constant
    * @param[in] Kd Differential constant
    */
   void setTunings(double Kp, double Ki, double Kd) {
      if ((Kp<0) || (Ki<0) || (Kd<0)) {
         USBDM::setAndCheckErrorCode(USBDM::E_ILLEGAL_PARAM);
      }
      kp = Kp;
      ki = Ki * interval;
      kd = Kd / interval;
   }

//   /**
//    * Change set-point of controller
//    *
//    * @param[in] value Value to set
//    */
//   void setSetpoint(double value) {
//      setpoint = value;
//   }
//
//   /**
//    * Get setpoint of controller
//    *
//    * @return Current setpoint
//    */
//   double getSetpoint() {
//      return setpoint;
//   }
//
   /**
    * Get input of controller
    *
    * @return Last input sample
    */
   double getInput() {
      return currentInput;
   }

   /**
    * Get setpoint of controller
    *
    * @return Last output sample
    */
   double getOutput() {
      return currentOutput;
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
    * Get proportional control factor
    *
    * @return factor as double
    */
   double getKp() {
      return  kp;
   }
   /**
    * Get integral control factor
    *
    * @return factor as double
    */
   double getKi() {
      return  ki/interval;
   }
   /**
    * Get differential control factor
    *
    * @return factor as double
    */
   double getKd() {
      return  kd*interval;
   }

   /**
    * Main PID calculation
    *
    * Should be executed at interval period
    */
   double newSample(double setpoint, double sample) {
//      PulseTp tp;

      if(!enabled) {
         return 0;
      }

      tickCount++;

      // Update input samples & error
      lastInput    = currentInput;
      currentInput = sample;
      currentError = setpoint - currentInput;

      integral += (ki * currentError);
      if(integral > outMax) {
         integral = outMax;
      }
      else if(integral < outMin) {
         integral = outMin;
      }
      double deltaInput = (currentInput - lastInput);

      currentOutput = kp * currentError + integral - kd * deltaInput;
      if(currentOutput > outMax) {
         currentOutput = outMax;
      }
      else if(currentOutput < outMin) {
         currentOutput = outMin;
      }
      // Update output
      return currentOutput;
   }
};

#endif // PROJECT_HEADERS_PID_H_
