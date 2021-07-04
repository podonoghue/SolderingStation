/**
 * @file    Pid.h
 * @brief   PID Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#include "PidControllerController.h"
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
         fIntegral     = fCurrentOutput;
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
double PidController::newSample(float targetTemperature, float actualTemperature) {

   const double lastInput = fCurrentInput;

   // Save for next iteration
   fCurrentInput = actualTemperature;

   if(!fEnabled) {
      return fCurrentOutput;
   }

   fTickCount++;

   // Update input samples & error
   fCurrentError = targetTemperature - fCurrentInput;

   fIntegral += (fKi * fCurrentError);

   // Limit integral term
   if(fIntegral > fILimit) {
      fIntegral = fILimit;
   }
   else if(fIntegral < -fILimit) {
      fIntegral = -fILimit;
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

      console.setFloatFormat(3, Padding_LeadingSpaces, 3);
      console.
         write("Kp = ").write(ch.getTip()->getKp()).write(',').
         write("Drive").write(',').
         write(ch.getTipName()).write(',').
         write("Inst. Temp").write(',').
         write("Error").write(',').
         write("P").write(',').
         write("I").write('<').write(ch.getTip()->getILimit()).write(',').
         write("D").
         writeln();

      console.
         write("Ki = ").writeln(ch.getTip()->getKi());

      console.
         write("Kd = ").writeln(ch.getTip()->getKd());
}

/**
 * Report current situation
 */
void PidController::report(Channel &ch) {

   console.setFloatFormat(1, Padding_LeadingSpaces, 4);
   console.write(getElapsedTime()).write(", ").write(fCurrentOutput).write(", ").write(fCurrentInput).write(", ").write(ch.tipTemperature.getLastSample()/50);

   console.setFloatFormat(2, Padding_LeadingSpaces, 5);
   console.write(",").write(fCurrentError).write(",").write(fProportional).write(",").write(fIntegral).write(",").write(fDifferential);

   console.writeln();
   console.resetFormat();
}
