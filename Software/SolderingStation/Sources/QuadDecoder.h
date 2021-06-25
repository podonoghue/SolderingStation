/*
 * Encoder.h
 *
 *  Created on: 15 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_ENCODER_H_
#define SOURCES_ENCODER_H_

class QuadDecoder {

private:

   /// Variable used by callback to track encoder position
   volatile int position;

   /// This pointer for static callback function
   static QuadDecoder *This;

   QuadDecoder(const QuadDecoder &other) = delete;
   QuadDecoder(QuadDecoder &&other) = delete;
   QuadDecoder& operator=(const QuadDecoder &other) = delete;
   QuadDecoder& operator=(QuadDecoder &&other) = delete;

public:
   /**
    * Constructor
    */
   QuadDecoder(){
      usbdm_assert(This == nullptr, "QuadDecoder instantiated more than once");
      This = this;
   }

   /**
    * Destructor
    */
   ~QuadDecoder() {}

   /**
    * Pin IRQ call-back.
    * Used to monitor shaft movements.\n
    * This uses a state machine that is resistant to contact bounce.
    *
    * @note Assumes 4 transitions/detent
    *
    * @param eventMask Mask indicating active pins
    */
   void pinIrqCallback(uint32_t eventMask);

   /**
    * Enable shaft encoder interface.
    *
    * Enables encoder interrupts and does any other initialisation required.
    */
   void initialise();

   /**
    * Returns the encoder position.
    */
   int getPosition();
};

#endif /* SOURCES_ENCODER_H_ */
