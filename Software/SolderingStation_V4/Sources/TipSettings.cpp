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
#include "T12.h"
#include "WellerWT50.h"

using namespace USBDM;

const InitialTipInfo TipSettings::initialTipInfo[NUMBER_OF_TIPS] = {
      {"B0",    IronType_T12 },
      {"B1",    IronType_T12 },
      {"B2",    IronType_T12 },
      {"B3",    IronType_T12 },
      {"B4",    IronType_T12 },
      {"B2Z",   IronType_T12 },
      {"BC1",   IronType_T12 },
      {"BC1.5", IronType_T12 },
      {"BC2",   IronType_T12 },
      {"BC3",   IronType_T12 },
      {"BC1Z",  IronType_T12 },
      {"BC2Z",  IronType_T12 },
      {"BC4Z",  IronType_T12 },
      {"BCF1",  IronType_T12 },
      {"BCF2",  IronType_T12 },
      {"BCF3",  IronType_T12 },
      {"BCF4",  IronType_T12 },
      {"BCF1Z", IronType_T12 },
      {"BCF2Z", IronType_T12 },
      {"BCF3Z", IronType_T12 },
      {"BCM2",  IronType_T12 },
      {"BCM3",  IronType_T12 },
      {"BL",    IronType_T12 },
      {"BZ",    IronType_T12 },
      {"C1",    IronType_T12 },
      {"C2",    IronType_T12 },
      {"C3",    IronType_T12 },
      {"C4",    IronType_T12 },
      {"D08",   IronType_T12 },
      {"D12",   IronType_T12 },
      {"D16",   IronType_T12 },
      {"D24",   IronType_T12 },
      {"D52",   IronType_T12 },
      {"D4",    IronType_T12 },
      {"DL12",  IronType_T12 },
      {"DL32",  IronType_T12 },
      {"DL52",  IronType_T12 },
      {"D12Z",  IronType_T12 },
      {"D24Z",  IronType_T12 },
      {"D4Z",   IronType_T12 },
      {"I",     IronType_T12 },
      {"IL",    IronType_T12 },
      {"ILS",   IronType_T12 },
      {"J02",   IronType_T12 },
      {"JL02",  IronType_T12 },
      {"JS02",  IronType_T12 },
      {"K",     IronType_T12 },
      {"KF",    IronType_T12 },
      {"KL",    IronType_T12 },
      {"KR",    IronType_T12 },
      {"KFZ",   IronType_T12 },
      {"KRZ",   IronType_T12 },
      {"KU",    IronType_T12 },
      {"WB2",   IronType_T12 },
      {"WD08",  IronType_T12 },
      {"WD12",  IronType_T12 },
      {"WD16",  IronType_T12 },
      {"WD52",  IronType_T12 },
      {"WI",    IronType_T12 },
      {"N1-06", IronType_T12 },
      {"N1-08", IronType_T12 },
      {"N1-10", IronType_T12 },
      {"N1-13", IronType_T12 },
      {"N1-16", IronType_T12 },
      {"N1-20", IronType_T12 },
      {"N1-23", IronType_T12 },
      {"N1-L1", IronType_T12 },
      {"WT50S", IronType_Weller },
      {"WT50M", IronType_Weller },
      {"WT50L", IronType_Weller },
};

/**
 * Get TipNameIndex for given tip name
 *
 * @param name  Tip name
 *
 * @return TipNameIndex (index into tip name table)
 */
TipSettings::TipNameIndex TipSettings::getTipNameIndex(const char *tipName) {
   static_assert(NUMBER_OF_TIPS == (sizeof(initialTipInfo)/sizeof(initialTipInfo[0])), "");

   TipNameIndex index;
   for(index=0; index<NUMBER_OF_TIPS; index++) {
      if (strcmp(initialTipInfo[index].name, tipName) == 0) {
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
   return initialTipInfo[index].name;
}

/**
 * Load default calibration and control values
 *
 * @param TipNameIndex  Tip name index for this setting
 */
void TipSettings::loadDefaultCalibration(TipNameIndex tipNameIndex) {
   this->tipNameIndex = tipNameIndex;
   this->flags   = 0;

   const InitialTipInfo &initInfo = initialTipInfo[tipNameIndex];
   switch(initInfo.type) {
      case IronType_T12:
         T12::initialiseSettings(this,  initInfo);
         break;
      case IronType_Weller:
         Weller_WT50::initialiseSettings(this,  initInfo);
         break;
      default:
         usbdm_assert(false, "Illegal iron type");
         break;
   }
}
