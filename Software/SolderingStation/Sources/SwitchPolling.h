/*
 * Switches.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_SWITCHPOLLING_H_
#define SOURCES_SWITCHPOLLING_H_

#include "Peripherals.h"
#include "EventQueue.h"
#include "QuadDecoder.h"

enum EventType : uint8_t {
   ev_None         ,

   ev_QuadPress    ,
   ev_QuadRelease  = ev_QuadPress + 1,
   ev_QuadHold     = ev_QuadPress + 2,

   ev_Ch1Press     ,
   ev_Ch1Release   = ev_Ch1Press + 1,
   ev_Ch1Hold      = ev_Ch1Press + 2,

   ev_Ch2Press     ,
   ev_Ch2Release   = ev_Ch2Press + 1,
   ev_Ch2Hold      = ev_Ch2Press + 2,

   ev_SelPress     ,
   ev_SelRelease   = ev_SelPress + 1,
   ev_SelHold      = ev_SelPress + 2,

   ev_Ch1Ch2Press  ,
   ev_Ch1Ch2Release   = ev_Ch1Ch2Press + 1,
   ev_Ch1Ch2Hold      = ev_Ch1Ch2Press + 2,

   ev_QuadRotate   ,

   ev_Tool1Active  ,
   ev_Tool2Active  = ev_Tool1Active+1,

   ev_Tool1Idle    ,
   ev_Tool2Idle    = ev_Tool1Idle+1,

   ev_Tool1LongIdle,
   ev_Tool2LongIdle = ev_Tool1LongIdle+1,
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

private:

   EventType pollSwitches();
   EventType pollSetbacks();

   /// Quadrature decode for rotary encoder
   Quadecoder encoder;

   /// Queue of pending events
   EventQueue<EventType, ev_None, 10> eventQueue;

   /// Static handle on class for timer call-back
   static SwitchPolling *This;

public:
   Event getEvent();

   SwitchPolling() {
      This = this;
   }

   /**
    * Initialise the switch polling
    */
   void initialise();

};

extern SwitchPolling switchPolling;

#endif /* SOURCES_SWITCHPOLLING_H_ */
