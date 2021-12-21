/*
 * StepResponseDriver.h
 *
 *  Created on: 22 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_STEPRESPONSEDRIVER_H_
#define SOURCES_STEPRESPONSEDRIVER_H_

class Channel;

class StepResponseDriver {

   StepResponseDriver(const StepResponseDriver &other) = delete;
   StepResponseDriver(StepResponseDriver &&other) = delete;
   StepResponseDriver& operator=(const StepResponseDriver &other) = delete;
   StepResponseDriver& operator=(StepResponseDriver &&other) = delete;

   Channel &channel;

public:
   /**
    * Constructor
    *
    * @param channel  Associated channel
    */
   StepResponseDriver(Channel &ch);

   ~StepResponseDriver() {}

   /**
    * Run step sequence
    *
    * @param maxDrive Drive to apply during step
    *
    * @return true  Completed successfully
    * @return false Failed
    */
   bool run(unsigned maxDrive);

};

#endif /* SOURCES_STEPRESPONSEDRIVER_H_ */
