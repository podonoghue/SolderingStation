/**
 * @file    Pid.h
 * @brief   PID Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#include "error.h"
#include "Channel.h"
#include "Pid.h"

/**
 * Set parameters being used
 *
 * @param[in] interval  Sample interval for controller in seconds
 * @param[in] min       Minimum value of output variable
 * @param[in] max       Maximum value of output variable
 */
void Pid::setParameters(float newInterval, float min, float max) {
   interval = newInterval;
   outMin   = min;
   outMax   = max;
}

/**
 * Set control parameters
 *
 * @param kp
 * @param ki
 * @param kd
 * @param iLimit
 */
void Pid::setControlParameters(float kp, float ki, float kd, float iLimit) {
   this->kp       = kp;
   this->ki       = ki * interval;
   this->kd       = kd / interval;
   this->iLimit   = iLimit;
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
double Pid::newSample(double setpoint, double sample) {

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
   if(integral > iLimit) {
      integral = iLimit;
   }
   else if(integral < -iLimit) {
      integral = -iLimit;
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

void Pid::report() {
   USBDM::console.setFloatFormat(2, USBDM::Padding_LeadingSpaces, 5);
   USBDM::console.write(",").write(currentError).write(",").write(proportional).write(",").write(integral).write(",").writeln(differential);
   USBDM::console.resetFormat();
}
