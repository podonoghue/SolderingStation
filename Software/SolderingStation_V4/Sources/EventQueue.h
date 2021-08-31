/*
 * Queue.h
 *
 *  Created on: 11 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_QUEUE_H_
#define SOURCES_QUEUE_H_
#include "system.h"

/**
 * Specialised queue
 * Takes advantage of a know empty value token
 *
 * These are discarded on add and automatically returned on empty queue.
 *
 * @tparam T            Type for queue elements
 * @tparam emptyValue   Special token indicating empty
 * @tparam size         Size of queue
 */
template<typename T, T emptyValue, unsigned size>
class EventQueue {
   T queue[size];
   unsigned head  = 0;
   unsigned tail  = 0;
   unsigned count = 0;

   EventQueue(const EventQueue &other) = delete;
   EventQueue(EventQueue &&other) = delete;
   EventQueue& operator=(const EventQueue &other) = delete;
   EventQueue& operator=(EventQueue &&other) = delete;

public:
   EventQueue() {}
   ~EventQueue() {}

   /**
    * Indicates if queue is empty
    *
    * @return true  => empty
    * @return false => not empty
    */
   bool isEmpty() {
      return count == 0;
   }

   /**
    * Indicates if queue is full
    *
    * @return true  => full
    * @return false => not full
    */
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
