/*
 * Channels.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_CHANNELS_H_
#define SOURCES_CHANNELS_H_

#include "PidController.h"
#include "Channel.h"
#include "Control.h"
#include "NonvolatileSettings.h"
#include "TakeBackHalfController.h"
#include "WellerWT50.h"
#include "T12.h"

/**
 * Holding class for the channels
 */
class Channels {

private:

   // Currently selected channel for front panel controls
   unsigned selectedChannel = 1;

//   using TempController = BangBangController;
   using TempController = PidController;
//   using TempController = TakeBackHalfController;

   TempController ch1Controller{Control::PID_INTERVAL, Control::MIN_DUTY, Control::MAX_DUTY};
   TempController ch2Controller{Control::PID_INTERVAL, Control::MIN_DUTY, Control::MAX_DUTY};

   Ch1SelectedLed ch1SelectedLed;
   Ch2SelectedLed ch2SelectedLed;

   Ch1Drive ch1Drive;
   Ch2Drive ch2Drive;

   // Channel1
   Channel channel1{nvinit.ch1Settings, ch1Controller, ch1SelectedLed, ch1Drive};

   // Channel2
   Channel channel2{nvinit.ch2Settings, ch2Controller, ch2SelectedLed, ch2Drive};

public:

   Channels() {
      using namespace USBDM;

      ch1Drive.setOutput();
      ch2Drive.setOutput();
      ch1SelectedLed.setOutput();
      ch2SelectedLed.setOutput();
   }

   /**
    * Number of channels
    */
   static constexpr unsigned NUM_CHANNELS = 2;

   /**
    * Get channel by channel number
    *
    * @param channelNumber
    *
    * @return Reference to indicate channel
    */
   Channel &operator[](int channelNumber) {
      usbdm_assert((channelNumber == 1)||(channelNumber == 2), "Illegal channel");

      return (channelNumber==1)?*(Channel*)&channel1:*(Channel*)&channel2;
   }

   /**
    * Get the number of the selected channel
    *
    * @return Channel number (zero indicates no currently selected channel)
    */
   unsigned getSelectedChannelNumber() {
      return selectedChannel;
   }

   /**
    * Get currently selected channel for front panel controls
    *
    * @return Reference to currently selected channel
    */
   Channel &getSelectedChannel() {
      return this->operator[](selectedChannel);
   }

   /**
    * Set currently selected channel for front panel controls
    * Restarts the idle counter for the selected channel
    *
    * @param channel Channel to make selected
    */
   void setSelectedChannel(unsigned channel) {
      selectedChannel = channel;
      getSelectedChannel().restartIdleTimer();
   }

   /**
    * Restart idle timers on all channels
    */
   void restartIdleTimers() {
      channel1.restartIdleTimer();
      channel2.restartIdleTimer();
   }
};

/// The channel information
extern Channels channels;

#endif /* SOURCES_CHANNELS_H_ */
