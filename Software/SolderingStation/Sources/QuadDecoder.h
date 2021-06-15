/*
 * Encoder.h
 *
 *  Created on: 15 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_ENCODER_H_
#define SOURCES_ENCODER_H_

class Quadecoder {

private:

   /// Variable used by callback to track encoder position
   volatile int position;

   /// This point for static callback function
   static Quadecoder *This;

   Quadecoder(const Quadecoder &other) = delete;
   Quadecoder(Quadecoder &&other) = delete;
   Quadecoder& operator=(const Quadecoder &other) = delete;
   Quadecoder& operator=(Quadecoder &&other) = delete;

public:
   /**
    * Constructor
    */
   Quadecoder(){
      This = this;
   }

   /**
    * Destructor
    */
   ~Quadecoder() {}

   /**
    * Pin IRQ call-back.
    * Used to monitor shaft movements.\n
    * This uses a state machine that is resistant to contact bounce.
    *
    * @note Assumes 4 transitions/detent
    *
    * @param eventMask Mask indicating active channel
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
