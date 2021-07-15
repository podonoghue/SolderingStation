/**
 * @file    Pid.h
 * @brief   PID Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#include "PidController.h"
#include "hardware.h"
#include "Channel.h"

using namespace USBDM;

/**
 * Set control parameters
 *
 * @param settings Parameter to use
 */
void PidController::setControlParameters(const TipSettings *settings) {
   this->fKp       = settings->getKp();
   this->fKi       = settings->getKi() * fInterval;
   this->fKd       = settings->getKd() / fInterval;
   this->fILimit   = settings->getILimit();
}

/**
 * Enable controller
 *
 * @note: Controller is re-initialised when enabled.
 * @note: Output is left unchanged when disabled.
 *
 * @param[in] enable True to enable
 */
void PidController::enable(bool enable) {
   if (enable) {
      if (!fEnabled) {
         // Just enabled
         fIntegral    = fCurrentOutput;
         fTickCount   = 0;
      }
   }
   fEnabled = enable;
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
float PidController::newSample(float targetTemperature, float actualTemperature) {

   const float lastInput = fCurrentInput;

   // Save for next iteration
   fCurrentInput = actualTemperature;

   if(!fEnabled) {
      // Assume manually set value
      return fCurrentOutput;
   }

   fTickCount++;

   // Update input samples & error
   fCurrentError = targetTemperature - fCurrentInput;

   fIntegral += (fKi * fCurrentError);
   if (fCurrentError>60) {
      // Limit integral term during startup when error is large
      if(fIntegral > fILimit/2) {
         fIntegral = fILimit/2;
      }
   }
   else {
      // Limit integral term after startup
      if(fIntegral > fILimit) {
         fIntegral = fILimit;
      }
      else if(fIntegral < -fILimit) {
         fIntegral = -fILimit;
      }
   }
   fDifferential = fKd * (fCurrentInput - lastInput);

   fProportional = fKp * fCurrentError;

   fCurrentOutput = fProportional + fIntegral - fDifferential;

   if(fCurrentOutput > fOutMax) {
      fCurrentOutput = fOutMax;
   }
   else if(fCurrentOutput < fOutMin) {
      fCurrentOutput = fOutMin;
   }

   // Update output
   return fCurrentOutput;
}

/**
 * Print heading for report()
 */
void PidController::reportHeading(Channel &ch) {

      console.setFloatFormat(1, Padding_None).
         write("Time,Drive,").write(ch.getTipName()).
         write(",Error,P=").write(getKp());
      console.setFloatFormat(3, Padding_None).
         write(",I=").write(getKi());
      console.setFloatFormat(1, Padding_None).
         write(" <").write(fILimit/2).write("/").write(fILimit).writeln(",D");

      console.setFloatFormat(3, Padding_None).
         write("\"Kp=").write(getKp()).
         write(",Ki=").write(getKi());
      console.setFloatFormat(3, Padding_None, 3).
         write(",Kd=").write(getKd());
      console.setFloatFormat(1, Padding_None).write(",I<").write(getIlimit()/2).write("/").write(getIlimit());
      console.write(",Pmax=").write((int)round(fOutMax)).write("%\"");
      console.writeln();
}

/**
 * Report current situation
 */
void PidController::report(Channel &) {

   // Take snapshot
   volatile float currentOutput = fCurrentOutput;
   volatile float currentInput  = fCurrentInput;
   volatile float currentError  = fCurrentError;
   volatile float proportional  = fProportional;
   volatile float differential  = fDifferential;
   volatile float integral      = fIntegral;
//   volatile float rawTipTemp    = ch.tipTemperature.getLastSample()/50; // Approximation!
//   volatile float resistance    = ch.tipTemperature.getResistance();

   console.setFloatFormat(2, Padding_LeadingSpaces, 3);
   console.write(getElapsedTime()).write(", ");

   console.setFloatFormat(1, Padding_LeadingSpaces, 3);
   console.write(currentOutput).write(", ").write(currentInput);
//   console.write(", ").write(rawTipTemp);

   console.setFloatFormat(1, Padding_LeadingSpaces, 3);
   console.write(",").write(currentError).write(",").write(proportional).write(",").write(integral).write(",").write(differential);

//   console.write(",").write(resistance);

   console.writeln();
   console.resetFormat();
}
