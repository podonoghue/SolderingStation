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
 * @param eventMask Mask indicating active channel
 */
void Quadecoder::pinIrqCallback(uint32_t eventMask) {

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
    * This matrix assumes 4 transitions/detent as shown
    * Expected encoder sequence CW = 00->01->11->10->00,  CCW = 00->10->11->01->00
    */
   static const DecoderState nextStateTable[][4] = {
      /* current : encoder -> 00               01           10           11         */
      /* DecoderState_Idle  */     { DecoderState_Idle,      DecoderState_CW01,  DecoderState_CCW10, DecoderState_Idle  },

      /* DecoderState_CW01  */     { DecoderState_Idle,      DecoderState_CW01,  DecoderState_Idle,  DecoderState_CW11  },
      /* DecoderState_CW11  */     { DecoderState_Idle,      DecoderState_CW01,  DecoderState_CW10,  DecoderState_CW11  },
      /* DecoderState_CW10  */     { DecoderState_IdleInc,   DecoderState_Idle,  DecoderState_CW10,  DecoderState_CW11  },

      /* DecoderState_CCW10 */     { DecoderState_Idle,      DecoderState_Idle,  DecoderState_CCW10, DecoderState_CCW11 },
      /* DecoderState_CCW11 */     { DecoderState_Idle,      DecoderState_CCW01, DecoderState_CCW10, DecoderState_CCW11 },
      /* DecoderState_CCW01 */     { DecoderState_IdleDec,   DecoderState_CCW01, DecoderState_Idle,  DecoderState_CCW11 },
   };

   static DecoderState currentState = DecoderState_Idle;

   // Check channel
   if (eventMask && QuadPhases::MASK) {
      // Use state of GPIOs to determine next state
      uint8_t value = QuadPhases::read();
      DecoderState nextState = nextStateTable[currentState][value];
      if (nextState&Inc) {
         position++;
      }
      else if (nextState&Dec) {
         position--;
      }
      currentState = (DecoderState)(nextState & 0x0F);
   }
}

/**
 * Enable shaft encoder interface.
 *
 * Enables encoder interrupts and does any other initialisation required.
 */
void Quadecoder::initialise() {

   // Start position at zero
   position = 0;

   /// Static call-back function
   static auto cb = [](unsigned long int pinMask) {
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
int Quadecoder::getPosition() {
   return position;
}

/// This pointer for static callback function
Quadecoder *Quadecoder::This = nullptr;
