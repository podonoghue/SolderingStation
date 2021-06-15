/*
 * Tips.h
 *
 *  Created on: 12 Jun. 2021
 *      Author: peter
 */

#ifndef SOURCES_TIPS_H_
#define SOURCES_TIPS_H_

class Tips {
private:
   static const char *tipNames[];

   Tips(const Tips &other) = delete;
   Tips(Tips &&other) = delete;
   Tips& operator=(const Tips &other) = delete;
   Tips& operator=(Tips &&other) = delete;

public:
   static constexpr unsigned NUMBER_OF_TIPS = 67;

   Tips() {}
   ~Tips() {}

   unsigned getTipIndex(const char *name);
   const char *getTipName(unsigned index);
};

#endif /* SOURCES_TIPS_H_ */
