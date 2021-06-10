/*
 * Channel.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "Channel.h"

const char *getChannelStateName(ChannelState state) {
   static const char *names[] = {
         "Off",
         "No Tip",
         "Over Ld",
         "BackOff",
         "Active",
   };
   if (state >= (sizeof(names)/sizeof(names[0]))) {
      return "???";
   }
   return names[state];
}

__attribute__ ((section(".flexRAM")))
ChannelSettings ch1Settings;

__attribute__ ((section(".flexRAM")))
ChannelSettings ch2Settings;
