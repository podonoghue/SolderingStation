/*
 * Channel.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "Channel.h"

/**
 * Get human readable name of channel state
 *
 * @param state State to describe
 *
 * @return Name of state
 */
const char *getChannelStateName(ChannelState state) {
   static const char *names[] = {
         "Off",
         "No Tip",
         "Over Ld",
         "Setback",
         "Active",
   };
   if (state >= (sizeof(names)/sizeof(names[0]))) {
      return "???";
   }
   return names[state];
}
