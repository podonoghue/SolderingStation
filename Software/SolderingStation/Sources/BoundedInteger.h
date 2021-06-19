/*
 * BoundedInteger.h
 *
 *  Created on: 17 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_BOUNDEDINTEGER_H_
#define SOURCES_BOUNDEDINTEGER_H_

template<int min, int max>
class BoundedInteger {

private:
   int value = 0;

public:
   BoundedInteger(int initialValue) : value(initialValue) {
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
