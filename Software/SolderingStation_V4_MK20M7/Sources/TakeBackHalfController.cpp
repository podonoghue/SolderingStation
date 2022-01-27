/**
 * TakeBackHalfController.h
 *
 * Modified from EEVBLOG discussion
 *   https://www.eevblog.com/forum/projects/yet-another-controller-for-jbc-t210t245/msg3106950/#msg3106950
 * See also:
 *   https://www.edn.com/temperature-controller-has-take-back-half-convergence-algorithm/
 *
 */
#include "TakeBackHalfController.h"
#include "hardware.h"
#include "Channel.h"

using namespace USBDM;

/**
 * Set control parameters
 *
 * @param settings Parameter to use
 */
void TakeBackHalfController::setControlParameters(const TipSettings *ts) {
   ts->getKd();
   fGamma = ts->getKp(); // 1.0;
   fBeta1 = ts->getKd()/fInterval; // 0.2;
   fBeta2 = 2*ts->getKd()/fInterval; // 0.4;
}

/**
 * Enable controller
 *
 * @note: Controller is re-initialised when enabled.
 * @note: Output is left unchanged when disabled.
 *
 * @param[in] enable True to enable
 */
void TakeBackHalfController::enable(bool enable) {
   if (enable) {
      if (!fEnabled) {
         // Just enabled
         fTickCount   = 0;
         //            lastInput        = initialTemperature;
         //            fTemperatureErrorPrevZC = targetTemperature - initialTemperature;
         fCurrentOutput          = 0.0;
         fCurrentOutputPrevZC    = 0.0;
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
float TakeBackHalfController::newSample(float targetTemperature, float actualTemperature) {

   fCurrentTarget = targetTemperature;

   const float lastInput = fCurrentInput;

   // Save for next iteration
   fCurrentInput = actualTemperature;

   if(!fEnabled) {
      return fCurrentOutput;
   }

   fTickCount++;

   fCurrentError = targetTemperature - actualTemperature;
   const float deltaTemp = (actualTemperature - lastInput);

   // Proportional term.
   fProportional = (fGamma * fCurrentError);

   fCurrentOutput += fProportional;

   // Improvement: use derivative to minimise overshoot/undershoot.
   if ((fCurrentError > 0.0) && (deltaTemp < 0.0)) {
      fDifferential = fBeta1 * deltaTemp;
   }
   else if ((fCurrentError < 0.0) && (deltaTemp > 0.0)) {
      fDifferential = fBeta2 * deltaTemp;
   }
   fCurrentOutput -= fDifferential;

   // Output variable clipping.
   if (fCurrentOutput < fOutMin) {
      fCurrentOutput = fOutMin;
   }
   else if (fCurrentOutput > fOutMax) {
      fCurrentOutput = fOutMax;
   }

   // Take-Back Half. (on zero crossing)
   if ((fCurrentError * fTemperatureErrorPrevZC) < 0.0) {
      fCurrentOutput = 0.5 * (fCurrentOutput + fCurrentOutputPrevZC);

      fTemperatureErrorPrevZC = fCurrentError;
      fCurrentOutputPrevZC    = fCurrentOutput;
   }

   // Update output
   return fCurrentOutput;
}

/**
 * Print heading for report()
 */
void TakeBackHalfController::reportHeading(Channel &ch) const {

   console.setFloatFormat(3, Padding_LeadingSpaces, 3);
   console.writeln("Target, \"Drive (", ch.getTipName(),")\nGamma = ", fGamma, "\nBeta1 = ", fBeta1, "\nBeta2 = ", fBeta2, "\", Inst. Temp, Error, differential");
}

/**
 * Report current situation
 */
void TakeBackHalfController::report() const {

   console.setFloatFormat(1, Padding_LeadingSpaces, 4);
   console.write(",", fCurrentTarget); // Set temperature
   console.write(",", fCurrentOutput); // Drive %
   console.write(",", fCurrentInput);  // Average temperature
   console.write(",", fCurrentError);  // Error
   console.write(",", fDifferential);  // Differential

}
