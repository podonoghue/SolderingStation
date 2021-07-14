/*
 * BangBangController.cpp
 *
 *  Created on: 6 Jul. 2021
 *      Author: peter
 */
#include "BangBangController.h"
#include "hardware.h"
#include "Channel.h"

using namespace USBDM;

/**
 * Set control parameters
 *
 * @param settings Parameter to use
 */
void BangBangController::setControlParameters(const TipSettings *) {
}

/**
 * Enable controller
 *
 * @note: Controller is re-initialised when enabled.
 * @note: Output is left unchanged when disabled.
 *
 * @param[in] enable True to enable
 */
void BangBangController::enable(bool enable) {
   if (enable) {
      if (!fEnabled) {
         // Just enabled
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
float BangBangController::newSample(float targetTemperature, float actualTemperature) {

   if(!fEnabled) {
      return 0;
   }

   fTickCount++;

   // Update input samples & error
   fLastInput    = fCurrentInput;
   fCurrentInput = actualTemperature;
   fCurrentError = targetTemperature - fCurrentInput;

   fCurrentOutput = fCurrentError<0?fOutMin:fOutMax;

   // Update output
   return fCurrentOutput;
}

/**
 * Print heading for report()
 */
void BangBangController::reportHeading(Channel &ch) {

      console.setFloatFormat(3, Padding_LeadingSpaces, 3);
      console.
         write("Time,Drive,").write(ch.getTipName()).
         writeln(",Inst. Temp,Error");
}

/**
 * Report current situation
 */
void BangBangController::report(Channel &) {

   // Take snapshot
   volatile float currentOutput = fCurrentOutput;
   volatile float currentInput  = fCurrentInput;
   volatile float currentError  = fCurrentError;
//   volatile float rawTipTemp    = ch.tipTemperature.getLastSample()/50; // Approximation!

   console.setFloatFormat(2, Padding_LeadingSpaces, 3);
   console.write(getElapsedTime()).write(", ");

   console.setFloatFormat(1, Padding_LeadingSpaces, 4);
   console.write(currentOutput).write(", ").write(currentInput);
//   console.write(", ").write(rawTipTemp);

   console.setFloatFormat(2, Padding_LeadingSpaces, 5);
   console.write(",").write(currentError);

   console.writeln();
   console.resetFormat();
}
