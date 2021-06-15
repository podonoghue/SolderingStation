/*
 * Tips.cpp
 *
 *  Created on: 12 Jun. 2021
 *      Author: peter
 */

#include <string.h>

#include "Tips.h"
#include "error.h"

using namespace USBDM;

const char *Tips::tipNames[NUMBER_OF_TIPS] = {
      "B0",
      "B1",
      "B2",
      "B3",
      "B4",
      "B2Z",
      "BC1",
      "BC1.5",
      "BC2",
      "BC3",
      "BC1Z",
      "BC2Z",
      "BC4Z",
      "BCF1",
      "BCF2",
      "BCF3",
      "BCF4",
      "BCF1Z",
      "BCF2Z",
      "BCF3Z",
      "BCM2",
      "BCM3",
      "BL",
      "BZ",
      "C1",
      "C2",
      "C3",
      "C4",
      "D08",
      "D12",
      "D16",
      "D24",
      "D52",
      "D4",
      "DL12",
      "DL32",
      "DL52",
      "D12Z",
      "D24Z",
      "D4Z",
      "I",
      "IL",
      "ILS",
      "J02",
      "JL02",
      "JS02",
      "K",
      "KF",
      "KL",
      "KR",
      "KFZ",
      "KRZ",
      "KU",
      "WB2",
      "WD08",
      "WD12",
      "WD16",
      "WD52",
      "WI",
      "N1-06",
      "N1-08",
      "N1-10",
      "N1-13",
      "N1-16",
      "N1-20",
      "N1-23",
      "N1-L1",
};

unsigned Tips::getTipIndex(const char *name) {
   static_assert(NUMBER_OF_TIPS == (sizeof(tipNames)/sizeof(tipNames[0])), "");
   unsigned index;
   for(index=0; index<10; index++) {
      if (strcmp(tipNames[index], name)) {
         break;
      }
   }
   usbdm_assert(index < NUMBER_OF_TIPS, "Tip not found");
   return index;
}

const char *Tips::getTipName(unsigned index) {

   usbdm_assert(index < NUMBER_OF_TIPS, "Illegal tip index");
   return tipNames[index];
}

