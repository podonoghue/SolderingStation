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
 * Call-back handling switch polling
 */
void SwitchPolling::callBack() {
   //=============================
   // Button polling
   //=============================
   static unsigned stableButtonCount = 0;
   static unsigned lastButtonPoll    = 0;

   unsigned currentButtonValue   = Buttons::read();

   // Stop counter rolling over
   if (stableButtonCount < HOLD_COUNT+1) {
      stableButtonCount++;
   }

   if (currentButtonValue != lastButtonPoll) {
      stableButtonCount = 0;
      lastButtonPoll    = currentButtonValue;
   }
   else {
      // Check at debounce time if active button
      if ((stableButtonCount == DEBOUNCE_COUNT) && currentButtonValue) {
         switch(currentButtonValue & (Buttons::MASK>>Buttons::RIGHT)) {
            case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))  : currentButton = ev_Ch1Press;    break;
            case (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : currentButton = ev_Ch2Press;    break;
            case (1<<(SelButton::BITNUM-Buttons::RIGHT))  : currentButton = ev_SelPress;    break;
            case (1<<(QuadButton::BITNUM-Buttons::RIGHT)) : currentButton = ev_QuadPress;   break;
            case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))|
                 (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : currentButton = ev_Ch1Ch2Press; break;
            default:
               break;
         }
         return;
      }
      // Check at hold time if active button
      if ((stableButtonCount == HOLD_COUNT) && currentButtonValue) {
         switch(currentButtonValue & (Buttons::MASK>>Buttons::RIGHT)) {
            case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))  : currentButton = ev_Ch1Hold;    break;
            case (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : currentButton = ev_Ch2Hold;    break;
            case (1<<(SelButton::BITNUM-Buttons::RIGHT))  : currentButton = ev_SelHold;    break;
            case (1<<(QuadButton::BITNUM-Buttons::RIGHT)) : currentButton = ev_QuadHold;   break;
            case (1<<(Ch1Button::BITNUM-Buttons::RIGHT))|
                 (1<<(Ch2Button::BITNUM-Buttons::RIGHT))  : currentButton = ev_Ch1Ch2Hold; break;
            default:
               break;
         }
         return;
      }
   }

   //=============================
   // Set-back Polling
   //=============================
   static unsigned toolStableCount = 0;
   static unsigned lastToolPoll    = 0;
   static unsigned tool1IdleCount  = 0;
   static unsigned tool2IdleCount  = 0;
   static bool     tool1Busy       = false;
   static bool     tool2Busy       = false;

   unsigned toolPoll = Setbacks::read();

   if (toolPoll != lastToolPoll) {
      toolStableCount = 0;
      lastToolPoll    = toolPoll;
      return;
   }
   else {
      // Check at debounce time
      if ((toolStableCount == DEBOUNCE_COUNT)) {
         tool1Busy = toolPoll & Ch1Stand::MASK;
         tool2Busy = toolPoll & Ch2Stand::MASK;
         if (tool1Busy) {
            // Tool out of holder - 1st removal
            currentButton  = ev_Tool1Active;
            return;
         }
         if (tool2Busy) {
            // Tool out of holder - 1st removal
            currentButton  = ev_Tool2Active;
            return;
         }
      }
      // Stop counter rolling over
      if (toolStableCount < DEBOUNCE_COUNT+1) {
         toolStableCount++;
      }
   }
   if (!tool1Busy) {
      // Tool in holder - increment idle time
      if (tool1IdleCount<LONGIDLE_MAX_COUNT+1) {
         tool1IdleCount++;
      }
      if (tool1IdleCount == IDLE_MAX_COUNT) {
         currentButton = ev_Tool1Idle;
         return;
      }
      else if (tool1IdleCount == LONGIDLE_MAX_COUNT) {
         currentButton = ev_Tool1LongIdle;
         return;
      }
   }
   if (!tool2Busy) {
      // Tool in holder - increment idle time
      if (tool2IdleCount<LONGIDLE_MAX_COUNT+1) {
         tool2IdleCount++;
      }
      if (tool2IdleCount == IDLE_MAX_COUNT) {
         currentButton = ev_Tool2Idle;
         return;
      }
      else if (tool2IdleCount == LONGIDLE_MAX_COUNT) {
         currentButton = ev_Tool2LongIdle;
         return;
      }
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

   Buttons::setInput(PinPull_Up, PinAction_None, PinFilter_None);
   Setbacks::setInput(PinPull_Up, PinAction_None, PinFilter_None);

   PollTimer::configureIfNeeded(PitDebugMode_Stop);
   auto channel = PollTimer::allocateChannel();
   PollTimer::configureChannel(channel, POLL_INTERVAL_IN_MS*ms, PitChannelIrq_Enabled);
   PollTimer::setCallback(channel, callBack);
   PollTimer::enableNvicInterrupts(channel, NvicPriority_Normal);

   currentButton = ev_None;
}


EventType   SwitchPolling::currentButton = ev_None;

