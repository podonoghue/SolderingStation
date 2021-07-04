/*
 * TipSettings.cpp
 *
 *  Created on: 12 Jun. 2021
 *      Author: peter
 */

#include <string.h>

#include "TipSettings.h"
#include "error.h"
#include "NonvolatileSettings.h"

using namespace USBDM;

const char * const TipSettings::tipNames[NUMBER_OF_TIPS] = {
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

/**
 * Get TipNameIndex for given tip name
 *
 * @param name  Tip name
 *
 * @return TipNameIndex (index into tip name table)
 */
TipSettings::TipNameIndex TipSettings::getTipNameIndex(const char *tipName) {
   static_assert(NUMBER_OF_TIPS == (sizeof(tipNames)/sizeof(tipNames[0])), "");

   TipNameIndex index;
   for(index=0; index<NUMBER_OF_TIPS; index++) {
      if (strcmp(tipNames[index], tipName) == 0) {
         return index;
      }
   }
   usbdm_assert(false, "Tip not found");
   return 0;
}

/**
 * Get tip name for given TipNameIndex
 *
 * @param index Index into name table
 *
 * @return Pointer to static string
 */
const char *TipSettings::getTipName(TipNameIndex index) {
   if (index >= NUMBER_OF_TIPS) {
      return "----";
   }
   return tipNames[index];
}
