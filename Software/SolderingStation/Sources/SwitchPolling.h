/*
 * Switches.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_SWITCHPOLLING_H_
#define SOURCES_SWITCHPOLLING_H_

#include "Peripherals.h"

enum EventType : uint16_t {
   ev_None         ,
   ev_QuadPress    ,
   ev_Ch1Press     ,
   ev_Ch2Press     ,
   ev_SelPress     ,
   ev_QuadRotate   ,
   ev_QuadHold     ,
   ev_Ch1Hold      ,
   ev_Ch2Hold      ,
   ev_SelHold      ,
   ev_Ch1Ch2Press  ,
   ev_Ch1Ch2Hold   ,
   ev_Tool1Active  ,
   ev_Tool1Idle    ,
   ev_Tool1LongIdle,
   ev_Tool2Active  ,
   ev_Tool2Idle    ,
   ev_Tool2LongIdle,
};

/**
 * Structure representing a front panel event such as
 * button press or knob rotation.
 */
struct Event {
   EventType   type;
   int16_t     change;
};

const char *getEventName(const EventType b);
const char *getEventName(const Event b);

/**
 * Timer driven class to represent the front panel.
 */
class SwitchPolling {

   static EventType currentButton;

   static int16_t getEncoder() {
      return QuadDecoder::getPosition();
   }

   static void callBack();

public:
   Event getEvent();

   SwitchPolling() {}

   /**
    * Initialise the switch polling
    */
   void initialise();
};

extern SwitchPolling switchPolling;

#endif /* SOURCES_SWITCHPOLLING_H_ */
