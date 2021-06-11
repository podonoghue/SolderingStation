/*
 * Switches.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include "SwitchPolling.h"
#include "Channels.h"
#include "queue"

using namespace USBDM;

/**
 * Number of consistent samples to confirm debouncing
 */
constexpr unsigned POLL_INTERVAL_IN_MS = 10;                               // Polled every 10 ms
constexpr unsigned DEBOUNCE_COUNT      = 40/POLL_INTERVAL_IN_MS;           // 40 ms
constexpr unsigned HOLD_COUNT          = 1*1000/POLL_INTERVAL_IN_MS;       // 1 s

/**
 * Get name of event from EventTYpe
 *
 * @param eventType Event type to describe
 *
 * @return Pointer to static string
 */
const char *getEventName(const EventType eventType) {
   static const char *table[] = {
         "ev_None",
         "ev_QuadPress",
         "ev_QuadRelease",
         "ev_QuadHold",
         "ev_Ch1Press",
         "ev_Ch1Release",
         "ev_Ch1Hold",
         "ev_Ch2Press",
         "ev_Ch2Release",
         "ev_Ch2Hold",
         "ev_SelPress",
         "ev_SelRelease",
         "ev_SelHold",
         "ev_Ch1Ch2Press",
         "ev_Ch1Ch2Release",
         "ev_Ch1Ch2Hold",
         "ev_QuadRotate",
         "ev_Tool1Active",
         "ev_Tool2Active",
         "ev_Tool1Idle",
         "ev_Tool2Idle",
         "ev_Tool1LongIdle",
         "ev_Tool2LongIdle",
   };
   if (eventType>=(sizeof(table)/sizeof(table[0]))) {
      return "???";
   }
   return table[eventType];
}

/**
 * Get name of event from Event
 *
 * @param event Event to describe
 *
 * @return Pointer to static string
 */
const char *getEventName(const Event event) {
   return getEventName(event.type);
}

/**
 * Button polling
 */
EventType SwitchPolling::pollSwitches() {

   // How long the buttons have been unchanged
   static unsigned stableButtonCount = 0;

   // Result from when the buttons were last polled
   static unsigned lastButtonPoll    = 0;

   static EventType lastEvent = ev_None;

   // Poll buttons
   unsigned  currentButtonValue = Buttons::read();

   // Count stable time with roll-over prevention
   if (stableButtonCount < UINT_MAX) {
      stableButtonCount++;
   }

   if (currentButtonValue != lastButtonPoll) {
      // Button change - start over
      stableButtonCount = 0;
      lastButtonPoll    = currentButtonValue;
      return ev_None;
   }

   if (currentButtonValue == 0) {
      if (lastEvent != ev_None) {
         EventType event = ((EventType)(lastEvent+1));
         lastEvent = ev_None;
         return event;
      }
      // No buttons pressed
      return ev_None;
   }

   // Check at debounce time for valid button
   if (stableButtonCount == DEBOUNCE_COUNT) {
//      console.write('d');

      // We have a button pressed for the debounce time - regular button press
      switch(currentButtonValue) {
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))  : lastEvent = ev_Ch1Press;    break;
         case (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : lastEvent = ev_Ch2Press;    break;
         case (1<<(SelButton::BITNUM-Buttons::RIGHT))  : lastEvent = ev_SelPress;    break;
         case (1<<(QuadButton::BITNUM-Buttons::RIGHT)) : lastEvent = ev_QuadPress;   break;
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))|
              (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : lastEvent = ev_Ch1Ch2Press; break;
         default:                                        lastEvent = ev_None;        break;
      }
      return lastEvent;
   }

   // Check at hold time for valid button
   if (stableButtonCount == HOLD_COUNT) {
//      console.write('l');
      EventType event = ev_None;

      // Don't record releases after hold
      lastEvent = ev_None;

      // We have a button pressed for the hold time - long held button
      switch(currentButtonValue) {
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))  : event = ev_Ch1Hold;    break;
         case (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : event = ev_Ch2Hold;    break;
         case (1<<(SelButton::BITNUM-Buttons::RIGHT))  : event = ev_SelHold;    break;
         case (1<<(QuadButton::BITNUM-Buttons::RIGHT)) : event = ev_QuadHold;   break;
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))|
              (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : event = ev_Ch1Ch2Hold; break;
         default:                                        event = ev_None;       break;
      }
      return event;
   }
   return ev_None;
}

/**
 * Set-back Polling
 */
EventType SwitchPolling::pollSetbacks() {

   // Indicates tool1 was in use
   static bool lastToolBusy[channels.NUM_CHANNELS] = {false};

   // Indicates tool is in use
   bool toolBusy[channels.NUM_CHANNELS];

   // Get current tool rest state
   unsigned toolPoll = Setbacks::read();

   // Poll tools
   toolBusy[0] = toolPoll & (1<<(Ch1Stand::BITNUM-Setbacks::RIGHT));
   toolBusy[1] = toolPoll & (1<<(Ch2Stand::BITNUM-Setbacks::RIGHT));

   for (unsigned tool=0; tool<channels.NUM_CHANNELS; tool++) {

      Channel &channel = channels[tool+1];

      if (lastToolBusy[tool] != toolBusy[tool]) {
         // Tool changed - start over
         channel.restartIdleTimer();
         lastToolBusy[tool]  = toolBusy[tool];
         if (toolBusy[tool]) {
            // Just started use of iron
            return static_cast<EventType>(ev_Tool1Active+tool);
         }
      }
      else if (!toolBusy[tool]) {

         // Tool in holder - increment idle time
         int idleTime = channel.incrementIdleTime(POLL_INTERVAL_IN_MS);

         if (idleTime == channel.nvSettings.backOffTime) {
            // Idle for a short while
            return static_cast<EventType>(ev_Tool1Idle+tool);
         }
         if (idleTime == channel.nvSettings.safetyOffTime) {
            // Idle for a long time
            return static_cast<EventType>(ev_Tool1LongIdle+tool);
         }
      }
   }
   return ev_None;
}

/**
 * Get last input event
 *
 * @return Event or ev_None if none.
 */
Event SwitchPolling::getEvent() {
   Event t = {ev_None, 0};

   t.type = eventQueue.get();

   if (t.type == ev_None) {
      static int16_t lastQuadPosition = 0;
      int16_t currentQuadPosition = getEncoder()/2;
      if (currentQuadPosition != lastQuadPosition) {
         t.type   = ev_QuadRotate;
         t.change = (currentQuadPosition - lastQuadPosition);
         lastQuadPosition = currentQuadPosition;
      }
   }
   return t;
}

/**
 * Initialise the switch polling
 */
void SwitchPolling::initialise() {
   QuadDecoder::configure(FtmPrescale_1, FtmQuadratureMode_Phase_AB_Mode);
   QuadDecoder::setInput(PinPull_Up, PinAction_None, PinFilter_Passive);
   QuadDecoder::enableFilter(7);

   Buttons::setInput(PinPull_Up, PinAction_None, PinFilter_Passive);
   Setbacks::setInput(PinPull_Up, PinAction_None, PinFilter_Passive);

   /**
    * Call-back handling switch polling
    */
   static const auto callBack = []() {
      This->eventQueue.add(This->pollSwitches());
      This->eventQueue.add(This->pollSetbacks());
   };

   PollingTimerChannel::configureIfNeeded(PitDebugMode_Stop);
   PollingTimerChannel::configure(POLL_INTERVAL_IN_MS*ms, PitChannelIrq_Enabled);
   PollingTimerChannel::setCallback(callBack);
   PollingTimerChannel::enableNvicInterrupts(NvicPriority_Normal);
}

SwitchPolling *SwitchPolling::This = nullptr;

