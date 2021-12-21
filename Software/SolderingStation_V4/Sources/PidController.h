/**
 * @file    PidController.h
 * @brief   PID Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#ifndef SOURCES_PIDCONTROLLER_H_
#define SOURCES_PIDCONTROLLER_H_

#include "Controller.h"

/**
 * PID Controller
 */
class PidController : public Controller {

private:
   float      fKp             = 0.0;
   float      fKi             = 0.0;
   float      fKd             = 0.0;
   float      fILimit         = 0.0;

   /// Integral accumulation term = sum(Ki * error(i)) * interval
   float      fIntegral       = 0.0;

   /// Differential term          = Kd/interval * (S(i)-S(i-1))
   float      fDifferential   = 0.0;

   /// Proportional term          = Kp * error(i)
   float      fProportional   = 0.0;

public:

   /**
    * Constructor
    *
    * @param[in] interval  Sample interval for controller in seconds
    * @param[in] outMin    Minimum output value
    * @param[in] outMax    Maximum output value
    */
   PidController(float interval, float outMin, float outMax) : Controller(interval, outMin, outMax) {
   }

   /**
   * Destructor
   */
   virtual ~PidController() {
   }

   /**
    * Get current Kp
    *
    * @return Value as float
    */
   float getKp() const {
      return fKp;
   }

   /**
    * Get current Ki
    *
    * @return Value as float
    */
   float getKi() const {
      return fKi / fInterval;
   }

   /**
    * Get current Kd
    *
    * @return Value as float
    */
   float getKd() const {
      return fKd * fInterval;
   }

   /**
    * Get current integration limit
    *
    * @return Value as float
    */
   float getIlimit() const {
      return fILimit;
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
    * @note: Controller may be re-initialised when enabled.
    * @note: Output is left unchanged when disabled.
    *
    * @param[in] enable True to enable
    */
   virtual void enable(bool enable = true);

   /**
    * Report current situation
    */
   virtual void report() const override ;

   /**
    * Print heading for report()
    */
   virtual void reportHeading(Channel &ch) const override ;
};

#endif // SOURCES_PIDCONTROLLER_H_
