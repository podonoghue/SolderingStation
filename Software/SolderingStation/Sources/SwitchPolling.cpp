/*
 * Switches.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */
#include <SwitchPolling.h>

using namespace USBDM;

/**
 * Number of consistent samples to confirm debouncing
 */
constexpr unsigned POLL_INTERVAL_IN_MS = 10;                               // Polled every 10 ms
constexpr unsigned DEBOUNCE_COUNT      = 50/POLL_INTERVAL_IN_MS;           // 50 ms
constexpr unsigned HOLD_COUNT          = 2*1000/POLL_INTERVAL_IN_MS;       // 2 s
constexpr unsigned IDLE_MAX_COUNT      = 5*60*1000/POLL_INTERVAL_IN_MS;    // 5 minutes
constexpr unsigned LONGIDLE_MAX_COUNT  = 10*60*1000/POLL_INTERVAL_IN_MS;   // 10 minutes

/**
 * Get name of event from EventTYpe
 *
 * @param eventType Event type to describe
 *
 * @return Pointer to static string
 */
const char *getEventName(const EventType eventType) {
   static const char *table[] = {
         "None",
         "QuadPress",
         "Ch1Press",
         "Ch2Press",
         "SelPress",
         "QuadRotate",
         "QuadHold",
         "Ch1Hold",
         "Ch2Hold",
         "SelHold",
         "Ch1Ch2Press",
         "Ch1Ch2Hold",
         "Tool1Active",
         "Tool1Idle",
         "Tool1LongIdle",
         "Tool2Active",
         "Tool2Idle",
         "Tool2LongIdle",
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

   // Poll buttons
   unsigned  currentButtonValue   = Buttons::read();

   // Stop counter rolling over
   if (stableButtonCount < HOLD_COUNT+1) {
      stableButtonCount++;
   }

   if (currentButtonValue != lastButtonPoll) {
      // Button change - start over
      stableButtonCount = 0;
      lastButtonPoll    = currentButtonValue;
      return ev_None;
   }

   // Check at debounce time if active button
   if ((stableButtonCount == DEBOUNCE_COUNT) && currentButtonValue) {
      // We have a button pressed for the debounce time - regular button press
      switch(currentButtonValue & (Buttons::MASK>>Buttons::RIGHT)) {
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))  : return ev_Ch1Press;    break;
         case (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : return ev_Ch2Press;    break;
         case (1<<(SelButton::BITNUM-Buttons::RIGHT))  : return ev_SelPress;    break;
         case (1<<(QuadButton::BITNUM-Buttons::RIGHT)) : return ev_QuadPress;   break;
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))|
              (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : return ev_Ch1Ch2Press; break;
         default:
            break;
      }
      return ev_None;
   }
   // Check at hold time if active button
   if ((stableButtonCount == HOLD_COUNT) && currentButtonValue) {
      // We have a button pressed for the hold time - long held button
      switch(currentButtonValue & (Buttons::MASK>>Buttons::RIGHT)) {
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))  : return ev_Ch1Hold;    break;
         case (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : return ev_Ch2Hold;    break;
         case (1<<(SelButton::BITNUM-Buttons::RIGHT))  : return ev_SelHold;    break;
         case (1<<(QuadButton::BITNUM-Buttons::RIGHT)) : return ev_QuadHold;   break;
         case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))|
              (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : return ev_Ch1Ch2Hold; break;
         default:
            break;
      }
      return ev_None;
   }
   return ev_None;
}

/**
 * Set-back Polling
 */
EventType SwitchPolling::pollSetbacks() {

   // How long tool1 has been idle
   static unsigned tool1IdleCount  = 0;

   // How long tool2 has been idle
   static unsigned tool2IdleCount  = 0;

   // Indicates tool1 was in use
   static bool     lastTool1Busy       = false;

   // Indicates tool2 was in use
   static bool     lastTool2Busy       = false;

   // Indicates tool1 is in use
   bool     tool1Busy       = false;

   // Indicates tool2 is in use
   bool     tool2Busy       = false;

   // Get current tool rest state
   unsigned toolPoll = Setbacks::read();

   tool1Busy = toolPoll & Ch1Stand::MASK;
   tool2Busy = toolPoll & Ch2Stand::MASK;

   if (lastTool1Busy != tool1Busy) {
      // Tool 1 changed - start over
      tool1IdleCount = 0;
      lastTool1Busy    = tool1Busy;
   }
   else if (!tool1Busy) {

      // Tool 1 in holder - increment idle time
      if (tool1IdleCount<LONGIDLE_MAX_COUNT+1) {
         tool1IdleCount++;
      }
      if (tool1IdleCount == IDLE_MAX_COUNT) {
         // Idle for a short while
         return ev_Tool1Idle;
      }
      if (tool1IdleCount == LONGIDLE_MAX_COUNT) {
         // Idle for a long time
         return ev_Tool1LongIdle;
      }
   }

   if (lastTool2Busy != tool2Busy) {
      // Changed - start over
      tool2IdleCount = 0;
      lastTool2Busy    = tool2Busy;
   }
   else if (!tool2Busy) {

      // Tool 2 in holder - increment idle time
      if (tool2IdleCount<LONGIDLE_MAX_COUNT+1) {
         tool2IdleCount++;
      }
      if (tool2IdleCount == IDLE_MAX_COUNT) {
         // Idle for a short while
         return ev_Tool2Idle;
      }
      if (tool2IdleCount == LONGIDLE_MAX_COUNT) {
         // Idle for a long time
         return ev_Tool2LongIdle;
      }
   }
   return ev_None;
}

/**
 * Call-back handling switch polling
 */
void SwitchPolling::callBack() {
   currentButton = pollSwitches();
   if (currentButton == ev_None) {
      currentButton = pollSetbacks();
   }
};

/**
 * Get last input event
 *
 * @return Event or ev_None if none.
 */
Event SwitchPolling::getEvent() {
   Event t = {ev_None, 0};
   if (currentButton != ev_None) {
      t.type = currentButton;
      currentButton = ev_None;
   }
   else {
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

   PollingTimerChannel::configureIfNeeded(PitDebugMode_Stop);
   PollingTimerChannel::configure(POLL_INTERVAL_IN_MS*ms, PitChannelIrq_Enabled);
   PollingTimerChannel::setCallback(callBack);
   PollingTimerChannel::enableNvicInterrupts(NvicPriority_Normal);

   currentButton = ev_None;
}


EventType   SwitchPolling::currentButton = ev_None;

