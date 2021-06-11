/*
 * Queue.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_QUEUE_H_
#define SOURCES_QUEUE_H_
#include "system.h"

template<typename T, T emptyValue, int size>
class Queue {
   T queue[size];
   unsigned head  = 0;
   unsigned tail  = 0;
   unsigned count = 0;

   Queue(const Queue &other) = delete;
   Queue(Queue &&other) = delete;
   Queue& operator=(const Queue &other) = delete;
   Queue& operator=(Queue &&other) = delete;

public:
   Queue() {}
   ~Queue() {}

   bool isEmpty() {
      return count == 0;
   }

   bool isFull() {
      return count == size;
   }

   /**
    * Add item to queue
    * @note emptyValue will be discarded
    *
    * @param item
    */
   void add(T item) {
      // Discard empty values
      if (item == emptyValue) {
         return;
      }
      USBDM::CriticalSection cs;

      if (isFull()) {
         return;
      }
      count++;
      queue[tail++] = item;
      if (tail >= size) {
         tail = 0;
      }
   }

   /**
    * Get value from queue.
    *
    * @return Value from queue or emptyValue if empty
    */
   T get() {
      USBDM::CriticalSection cs;

      if (isEmpty()) {
         return emptyValue;
      }
      count--;
      T item = queue[head++];
      if (head >= size) {
         head = 0;
      }
      return item;
   }
};

#endif /* SOURCES_QUEUE_H_ */
