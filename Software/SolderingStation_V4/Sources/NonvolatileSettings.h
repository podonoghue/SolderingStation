/*
 * NonvolatileSettings.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_NONVOLATILESETTINGS_H_
#define SOURCES_NONVOLATILESETTINGS_H_

#include "flash.h"
#include "ChannelSettings.h"
#include "Tips.h"

/**
 * A derived class similar to this should be created to do the following:
 * - Wait for initialisation of the FlexRAM from the Flash backing store - Flash();
 * - Configure and partition the flash on the first reset after programming the device - initialiseEeprom().
 * - Do once-only initialisation of non-volatile variables when the above occurs.
 * - It may also be used as a singleton repository for all the non-voltile objects if desired.
 */
class NonvolatileSettings : public USBDM::Flash {

   friend class Control;
   friend class Channels;
   friend class Menus;
   friend class Tips;

private:

   ///  Channel 1 non-volatile settings
   ChannelSettings ch1Settings;

   ///  Channel 2 non-volatile settings
   ChannelSettings ch2Settings;

   /// Settings for tips selected as available
   Tips::TipSettingsArray tipSettings;

private:

   /**
    * Initialise a non-volatile channel settings
    */
   void initialiseChannelSettings();
   /**
    * Initialise all Tip non-volatile settings
    */
   void initialiseTipSettings();

   /**
    * Initialise Tip non-volatile settings
    */
   void initialiseTipSettings(TipSettings *tipSettings);

   NonvolatileSettings(const NonvolatileSettings &other) = delete;
   NonvolatileSettings(NonvolatileSettings &&other) = delete;
   NonvolatileSettings& operator=(const NonvolatileSettings &other) = delete;
   NonvolatileSettings& operator=(NonvolatileSettings &&other) = delete;

public:

   /**
    * Constructor
    */
   NonvolatileSettings();

   /**
    * Initialise non-volatile storage to default values
    */
   void initialiseNonvolatileStorage();

};

/// Actual non-volatile object
extern NonvolatileSettings nvinit;

#endif /* SOURCES_NONVOLATILESETTINGS_H_ */
