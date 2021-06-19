/*
 * PidSettings.h
 *
 *  Created on: 12 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_PIDSETTINGS_H_
#define SOURCES_PIDSETTINGS_H_

#include "flash.h"

class PidSettings {

public:
   USBDM::Nonvolatile<float> kp;
   USBDM::Nonvolatile<float> ki;
   USBDM::Nonvolatile<float> kd;
   USBDM::Nonvolatile<float> iLimit;
};

#endif /* SOURCES_PIDSETTINGS_H_ */
