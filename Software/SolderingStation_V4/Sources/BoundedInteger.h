/*
 * BoundedInteger.h
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_BOUNDEDINTEGER_H_
#define SOURCES_BOUNDEDINTEGER_H_

class BoundedInteger {

private:
   const int min;
   const int max;
   int value;

public:
   /**
    * Constructor for a bounded integer
    *
    * @param min           Minimum value
    * @param max           Maximum value
    * @param initialValue  Initial value
    */
   BoundedInteger(int min, int max, int initialValue) : min(min), max(max), value(initialValue) {
   }

   /**
    * Constructor for a bounded integer
    * The minimum value defaults to zero
    *
    * @param max           Maximum value
    * @param initialValue  Initial value
    */
   BoundedInteger(int max, int initialValue) : min(0), max(max), value(initialValue) {
   }

   void limit() {
      if (value>max) {
         value = max;
      }
      if (value<min) {
         value = min;
      }
   }

   BoundedInteger &operator+=(int delta) {
      value += delta;
      limit();
      return *this;
   }

   BoundedInteger &operator-=(int delta) {
      value -= delta;
      limit();
      return *this;
   }

   BoundedInteger &operator++(int) {
      value++;
      limit();
      return *this;
   }

   BoundedInteger &operator--(int) {
      value--;
      limit();
      return *this;
   }

   operator int() {
      return value;
   }

   BoundedInteger &operator=(int other) {
      value = other;
      return *this;
   }
};

#endif /* SOURCES_BOUNDEDINTEGER_H_ */
