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
void TakeBackHalfController::setControlParameters(const TipSettings *) {
   fGamma = 1.0; // 1.0;
   fBeta1 = 0.2; // 0.2;
   fBeta2 = 0.4; // 0.4;
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

   const float lastInput = fCurrentInput;

   // Save for next iteration
   fCurrentInput = actualTemperature;

   if(!fEnabled) {
      return fCurrentOutput;
   }

   fTickCount++;

   const float fTemperatureError = targetTemperature - actualTemperature;
   const float fTemperatureDeriv = (actualTemperature - lastInput) / fInterval;

   // Integral term.
   fCurrentOutput += (fGamma * fTemperatureError);

   // Improvement: use derivative to minimise overshoot/undershoot.
   if ((fTemperatureError > 0.0) && (fTemperatureDeriv < 0.0)) {
      fCurrentOutput -= fBeta1 * fTemperatureDeriv;
   }
   else if ((fTemperatureError < 0.0) && (fTemperatureDeriv > 0.0)) {
      fCurrentOutput -= fBeta2 * fTemperatureDeriv;
   }

   // Output variable clipping.
   if (fCurrentOutput < fOutMin) {
      fCurrentOutput = fOutMin;
   }
   else if (fCurrentOutput > fOutMax) {
      fCurrentOutput = fOutMax;
   }

   // Take-Back Half.
   if ((fTemperatureError * fTemperatureErrorPrevZC) < 0.0) {
      fCurrentOutput = 0.5 * (fCurrentOutput + fCurrentOutputPrevZC);

      fTemperatureErrorPrevZC = fTemperatureError;
      fCurrentOutputPrevZC    = fCurrentOutput;
   }

   // Update output
   return fCurrentOutput;
}

/**
 * Print heading for report()
 */
void TakeBackHalfController::reportHeading(Channel &ch) {

      console.setFloatFormat(3, Padding_LeadingSpaces, 3);
      console.
         write("Gamma = ").write(fGamma).write(',').
         write("Drive").write(',').
         write(ch.getTipName()).write(',').
         write("Inst. Temp").
         writeln();

      console.
         write("Beta1 = ").write(fBeta1).writeln();

      console.
         write("Beta2 = ").write(fBeta2).writeln();
}

/**
 * Report current situation
 */
void TakeBackHalfController::report(Channel &) {

   console.setFloatFormat(1, Padding_LeadingSpaces, 4);
//   console.write(getElapsedTime()).write(", ").write(fCurrentOutput).write(", ").write(fCurrentInput).write(", ").write(ch.tipTemperature.getLastSample()/50);

//   console.setFloatFormat(2, Padding_LeadingSpaces, 5);
//   console.write(",").write(fCurrentError).write(",").write(fProportional).write(",").write(fIntegral).write(",").write(fDifferential);

   console.writeln();
   console.resetFormat();
}
