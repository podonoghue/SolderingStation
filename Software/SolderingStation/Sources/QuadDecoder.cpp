/*
 * Encoder.cpp
 *
 *  Created on: 15 Jun. 2021
 *      Author: peter
 */

#include "hardware.h"
#include "Peripherals.h"
#include "QuadDecoder.h"

using namespace USBDM;

/**
 * Pin IRQ call-back.
 * Used to monitor shaft movements.\n
 * This uses a state machine that is resistant to contact bounce.
 *
 * @note Assumes 4 transitions/detent
 *
    * @param eventMask Mask indicating active pins
 */
void QuadDecoder::pinIrqCallback(uint32_t eventMask) {

   // Count actions encoded in state
   constexpr uint8_t Inc = 0b010000;
   constexpr uint8_t Dec = 0b100000;

   enum DecoderState : uint8_t {
      DecoderState_Idle,
      DecoderState_CW01,
      DecoderState_CW11,
      DecoderState_CW10,
      DecoderState_CCW10,
      DecoderState_CCW11,
      DecoderState_CCW01,
      DecoderState_IdleInc  = DecoderState_Idle|Inc,  // Idle + increment count
      DecoderState_IdleDec  = DecoderState_Idle|Dec,  // Idle + decrement count
   };

   /**
    * Next state matrix: nextStateTable[current state][current input] -> next state
    * This matrix assumes 4 transitions/detent.
    * Expected encoder sequence CCW = 00->01->11->10->00,  CW = 00->10->11->01->00.
    * Swap DecoderState_IdleInc/DecoderState_IdleDec to reverse.
    */
   static const DecoderState nextStateTable[][4] = {
      /*     Current                <------------------------------ Encoder phase outputs----------------------------->  */
      /*      state                         00                      01                  10                  11           */
      /* DecoderState_Idle  */     { DecoderState_Idle,      DecoderState_CW01,  DecoderState_CCW10, DecoderState_Idle  },

      /* DecoderState_CW01  */     { DecoderState_Idle,      DecoderState_CW01,  DecoderState_Idle,  DecoderState_CW11  },
      /* DecoderState_CW11  */     { DecoderState_Idle,      DecoderState_CW01,  DecoderState_CW10,  DecoderState_CW11  },
      /* DecoderState_CW10  */     { DecoderState_IdleInc,   DecoderState_Idle,  DecoderState_CW10,  DecoderState_CW11  },

      /* DecoderState_CCW10 */     { DecoderState_Idle,      DecoderState_Idle,  DecoderState_CCW10, DecoderState_CCW11 },
      /* DecoderState_CCW11 */     { DecoderState_Idle,      DecoderState_CCW01, DecoderState_CCW10, DecoderState_CCW11 },
      /* DecoderState_CCW01 */     { DecoderState_IdleDec,   DecoderState_CCW01, DecoderState_Idle,  DecoderState_CCW11 },
   };

   static DecoderState currentState = DecoderState_Idle;

   // Check interrupt channel
   if (eventMask && QuadPhases::MASK) {

      // Use current state and state of GPIOs to determine next state
      DecoderState nextState = nextStateTable[currentState][QuadPhases::read()];

      // Do transition actions
      if (nextState&Inc) {
         position++;
      }
      else if (nextState&Dec) {
         position--;
      }
      // Strip actions from state
      currentState = (DecoderState)(nextState & 0x0F);
   }
}

/**
 * Enable shaft encoder interface.
 *
 * Enables encoder interrupts and does any other initialisation required.
 */
void QuadDecoder::initialise() {

   // Start position at zero
   position = 0;

   // Static call-back function
   static auto cb = [](uint32_t pinMask) {
      This->pinIrqCallback(pinMask);
   };

   // Configure encoder pins as inputs with dual-edge interrupts
   QuadPhases::setCallback(cb);
   QuadPhases::setInput(PinPull_Up, PinAction_IrqEither, PinFilter_Passive);
   QuadPhases::enableNvicInterrupts(NvicPriority_Normal);
}

/**
 * Returns the encoder position.
 */
int QuadDecoder::getPosition() {
   return position;
}

/// This pointer for static callback function
QuadDecoder *QuadDecoder::This = nullptr;
