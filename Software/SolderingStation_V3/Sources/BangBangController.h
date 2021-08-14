/**
 * @file    BangBang.h
 * @brief   Bang-Bang Controller class
 *
 *  Created on: 10 Jul 2021
 *      Author: podonoghue
 */
#ifndef SOURCES_BANGBANG_H_
#define SOURCES_BANGBANG_H_

#include "Controller.h"

/**
 * BangBang Controller
 */
class BangBangController : public Controller {

   float   fLastInput = 0;

public:

   /**
    * Constructor
    *
    * @param[in] interval  Sample interval for controller in seconds
    * @param[in] outMin    Minimum output value
    * @param[in] outMax    Maximum output value
    */
   BangBangController(float interval, float outMin, float outMax) : Controller(interval, outMin, outMax) {
   }

   /**
   * Destructor
   */
   virtual ~BangBangController() {
   }

   /**
    * Set control parameters
    *
    * @param settings Parameter to use
    */
   virtual void setControlParameters(const TipSettings *settings);

   /**
    * Enable controller\n
    * Note: Controller is re-initialised when enabled
    *
    * @note: Controller is re-initialised when enabled.
    * @note: Output is left unchanged when disabled.
    *
    * @param[in] enable True to enable
    */
   virtual void enable(bool enable = true);

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
   virtual float newSample(float targetTemperature, float actualTemperature);
   /**
    * Report current situation
    */
   virtual void report(Channel &ch);

   /**
    * Print heading for report()
    */
   virtual void reportHeading(Channel &ch);
};

#endif // SOURCES_BANGBANG_H_
