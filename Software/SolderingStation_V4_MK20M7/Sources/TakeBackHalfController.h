/**
 * TakeBackHalfController.h
 *
 * Modified from EEVBLOG discussion
 *   https://www.eevblog.com/forum/projects/yet-another-controller-for-jbc-t210t245/msg3106950/#msg3106950
 * See also:
 *   https://www.edn.com/temperature-controller-has-take-back-half-convergence-algorithm/
 *
 */
#ifndef SOURCES_TAKEBACKHALFCONTROLLER_H_
#define SOURCES_TAKEBACKHALFCONTROLLER_H_

#include "Controller.h"

/**
 * Improved Take-Back-Half algorithm.
 */
class TakeBackHalfController : public Controller {

private:
   // Proportional term
   float fGamma = 0;

   // Differential term
   float fBeta1 = 0;

   // Differential term
   float fBeta2 = 0;

   /// Temperature in previous zero crossing?
   float fTemperatureErrorPrevZC = 0;

   /// Output at previous zero crossing
   float fCurrentOutputPrevZC    = 0;

public:

   /**
    * Constructor
    *
    * @param[in] interval  Sample interval for controller in seconds
    * @param[in] outMin    Minimum output value
    * @param[in] outMax    Maximum output value
    */
   TakeBackHalfController(float interval, float outMin, float outMax) : Controller(interval, outMin, outMax) {
   }

   /**
   * Destructor
   */
   virtual ~TakeBackHalfController() {
   }

   /**
    * Set control parameters
    *
    * @param settings Parameter to use
    */
   virtual void setControlParameters(const TipSettings *settings) override ;

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
   virtual float newSample(float actualTemperature, float targetTemperature) override ;

   /**
    * Enable controller
    *
    * @note: Controller is re-initialised when enabled.
    * @note: Output is left unchanged when disabled.
    *
    * @param[in] enable True to enable
    */
   virtual void enable(bool enable = true) override ;

   /**
    * Report current situation
    */
   virtual void report() const override ;

   /**
    * Print heading for report()
    */
   virtual void reportHeading(Channel &ch) const override ;
};

#endif /* SOURCES_TAKEBACKHALFCONTROLLER_H_ */
