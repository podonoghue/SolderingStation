/*
 * NonvolatileSettingsX.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_NONVOLATILESETTINGS_H_
#define SOURCES_NONVOLATILESETTINGS_H_

#include "flash.h"
#include "ChannelSettings.h"

/**
 * A derived class similar to this should be created to do the following:
 * - Wait for initialisation of the FlexRAM from the Flash backing store - Flash();
 * - Configure and partition the flash on the first reset after programming the device - initialiseEeprom().
 * - Do once-only initialisation of non-volatile variables when the above occurs.
 * - It may also be used as a singleton repository for all the non-voltile objects if desired.
 */
class NonvolatileSettings : public USBDM::Flash {

   friend class Channels;

public:
   ///  Channel 1 non-volatile settings
   ChannelSettings ch1Settings;

   ///  Channel 2 non-volatile settings
   ChannelSettings ch2Settings;

private:

   void initialiseChannelSettings(ChannelSettings &settings);

   NonvolatileSettings(const NonvolatileSettings &other) = delete;
   NonvolatileSettings(NonvolatileSettings &&other) = delete;
   NonvolatileSettings& operator=(const NonvolatileSettings &other) = delete;
   NonvolatileSettings& operator=(NonvolatileSettings &&other) = delete;

public:
   NonvolatileSettings();

   /**
    * Initialise non-volatile storage to default values
    */
   void initialiseNonvolatileStorage();

};

/// Actual non-volatile object
extern NonvolatileSettings nvinit;

#endif /* SOURCES_NONVOLATILESETTINGS_H_ */
