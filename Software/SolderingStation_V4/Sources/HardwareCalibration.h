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
   USBDM::Nonvolatile<float> lowGainNoBoost;
   USBDM::Nonvolatile<float> lowGainWithBoost;
   USBDM::Nonvolatile<float> highGainNoBoost;
   USBDM::Nonvolatile<float> highGainWithBoost;

   HardwareCalibration() {}
   ~HardwareCalibration() {}

   void initialise() {
      vccValue          = 3.30;
      lowGainNoBoost    = LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF;
      lowGainWithBoost  = LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF;
      highGainNoBoost   = 1.05*HIGH_GAIN_MEASUREMENT_RATIO_BOOST_OFF;
      highGainWithBoost = 1.05*HIGH_GAIN_MEASUREMENT_RATIO_BOOST_ON;
   }
};

#endif /* SOURCES_HARDWARECALIBRATION_H_ */
