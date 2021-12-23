/*
 * HardwareCalibration.h
 *
 *  Created on: 20 Oct. 2021
 *      Author: peter
 */

#ifndef SOURCES_HARDWARECALIBRATION_H_
#define SOURCES_HARDWARECALIBRATION_H_

#include "flash.h"

class HardwareCalibration {
public:

   USBDM::Nonvolatile<float> vccValue;
   USBDM::Nonvolatile<float> preAmplifierNoBoost;
   USBDM::Nonvolatile<float> preAmplifierWithBoost;

   HardwareCalibration() {}
   ~HardwareCalibration() {}

   void initialise() {
      vccValue               = 3.30;
      preAmplifierNoBoost    = LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF;
      preAmplifierWithBoost  = LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF;
   }
};

#endif /* SOURCES_HARDWARECALIBRATION_H_ */
