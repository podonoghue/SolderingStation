/*
 * Channels.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_CHANNELS_H_
#define SOURCES_CHANNELS_H_

#include "Channel.h"
#include "NonvolatileSettings.h"

/**
 * Holding class for the channels
 */
class Channels {

public:

   /// Number of channels
   static constexpr unsigned NUM_CHANNELS = 2;

   /// Channel1 state information
   Channel  channel1 = Channel(nvinit.ch1Settings);

   /// Channel2 state information
   Channel  channel2 = Channel(nvinit.ch2Settings);

   /// Currently selected channel for front panel controls
   unsigned selectedChannel = 0;

   /**
    * Get channel by channel number
    *
    * @param channelNumber
    *
    * @return Reference to indicate channel
    */
   Channel &operator[](int channelNumber) {
      usbdm_assert((channelNumber == 1)||(channelNumber == 2), "Illegal channel");

      return (channelNumber==1)?channel1:channel2;
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
    *
    * @param channel Channel to make selected
    */
   void setSelectedChannel(unsigned channel) {
      selectedChannel = channel;
   }
};

/// The channel information
extern Channels channels;

#endif /* SOURCES_CHANNELS_H_ */
