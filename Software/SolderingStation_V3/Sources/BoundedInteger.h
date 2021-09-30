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

   /**
    * Limit value to acceptable range
    */
   void limit() {
      if (value>max) {
         value = max;
      }
      if (value<min) {
         value = min;
      }
   }

   /**
    * Add delta to current value
    *
    * @param delta Amount to increment value by
    *
    * @return  New current value
    */
   BoundedInteger &operator+=(int delta) {
      value += delta;
      limit();
      return *this;
   }

   /**
    * Subtract delta from current value
    *
    * @param delta Amount to decrement value by
    *
    * @return  New current value
    */
   BoundedInteger &operator-=(int delta) {
      value -= delta;
      limit();
      return *this;
   }

   /**
    * Add 1 to current value
    *
    * @return  New current value
    */
   BoundedInteger &operator++(int) {
      value++;
      limit();
      return *this;
   }

   /**
    * Subtract 1 from current value
    *
    * @return  New current value
    */
   BoundedInteger &operator--(int) {
      value--;
      limit();
      return *this;
   }

   /**
    * Get current value
    */
   operator int() {
      return value;
   }

   /**
    * Assign value
    *
    * @param value Value to assign
    *
    * @return  Current value (may differ from value)
    */
   BoundedInteger &operator=(int value) {
      this->value = value;
      limit();
      return *this;
   }
};

#endif /* SOURCES_BOUNDEDINTEGER_H_ */
