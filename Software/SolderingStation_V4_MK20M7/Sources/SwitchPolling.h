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
   ev_None,

   ev_QuadRelease,
   ev_QuadHold     = ev_QuadRelease + 1,

   ev_Ch1Release,
   ev_Ch1Hold      = ev_Ch1Release + 1,

   ev_Ch2Release,
   ev_Ch2Hold      = ev_Ch2Release + 1,

   ev_SelRelease,
   ev_SelHold      = ev_SelRelease + 1,

   ev_Ch1Ch2Release,
   ev_Ch1Ch2Hold      = ev_Ch1Ch2Release + 1,

   ev_QuadRotate,
   ev_QuadRotatePressed = ev_QuadRotate+1,
};

/**
 * Structure representing a front panel event such as
 * button press or knob rotation.
 */
class Event {

public:
   EventType   type;
   int16_t     change;

   Event() : type(ev_None), change(0) {
   }

   Event(EventType ev_, int16_t change) : type(ev_), change(change) {
   }

   /**
    * Indicates if the event is ev_SelHold or ev_QuadHold
    *
    * @return
    */
   bool isSelHold() {
      return (type == ev_SelHold) || (type == ev_QuadHold);
   }

   /**
    * Indicates if the event is ev_SelRelease or ev_QuadRelease
    *
    * @return
    */
   bool isSelRelease() {
      return (type == ev_SelRelease) || (type == ev_QuadRelease);
   }
};

const char *getEventName(const EventType b);

/**
 * Get name of event from Event
 *
 * @param event Event to describe
 *
 * @return Pointer to static string
 */
inline const char *getEventName(const Event event) {
   return getEventName(event.type);
}

/**
 * Timer driven class to represent the front panel.
 */
class SwitchPolling {

private:

   EventType pollSwitches();
   void      pollSetbacks();

   /// Quadrature decode for rotary encoder
   QuadDecoder encoder;

   /// Queue of pending events
   EventQueue<EventType, ev_None, 10> eventQueue;

   /// Static handle on class for timer call-back
   static SwitchPolling *This;

   enum QuadState {
      QuadState_Normal,          // Quad button not pressed
      QuadState_Pressed,         // Quad button currently pressed
      QuadState_Pressed_Rotate   // Quad button pressed and rotation detected
   };

   QuadState quadState = QuadState_Normal;

public:
   Event getEvent();

   SwitchPolling() {
      usbdm_assert(This == nullptr, "SwitchPolling instantiated more than once");
      This = this;
   }

   /**
    * Initialise the switch polling
    */
   void initialise();

};

extern SwitchPolling switchPolling;

#endif /* SOURCES_SWITCHPOLLING_H_ */
