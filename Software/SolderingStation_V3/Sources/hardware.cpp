/**
 * @file      hardware.cpp (generated from MK20D5.usbdmHardware)
 * @version   1.3.0
 * @brief     Pin initialisation for MK20DX128VLF5
 *
 * *****************************
 * *** DO NOT EDIT THIS FILE ***
 * *****************************
 *
 * This file is generated automatically.
 * Any manual changes will be lost.
 */

#include "hardware.h"

/**
 * Namespace enclosing USBDM classes
 */
namespace USBDM {

/**
 * @addtogroup USBDM_Group USBDM Peripheral Interface
 * @brief Hardware Peripheral Interface and library
 * @{
 */

/**
 * Startup code for C++ classes
 */
extern "C" void __attribute__((constructor)) cpp_initialise() {
   if constexpr (MapAllPinsOnStartup) {
      mapAllPins();
   }
}

/// Channel 1 drive (Bit Field)
const GpioFieldTable_T<GpioCInfo, 2, 1, ActiveLow>           ch1Drive;                                     // PTC1(p34), PTC2(p35)
/// Channel 1 selected LED
const GpioTable_T<GpioCInfo, 0, ActiveHigh>                  ch1SelectedLed;                               // PTC0(p33)
/// Channel 2 selected LED
const GpioTable_T<GpioCInfo, 6, ActiveHigh>                  ch2SelectedLed;                               // PTC6(p39)
/// Channel 2 drive (Bit Field)
const GpioFieldTable_T<GpioCInfo, 4, 3, ActiveLow>           ch2Drive;                                     // PTC3(p36), PTC4(p37)


/**
 * Map all configured pins to peripheral signals.
 *
 * PCRs of allocated pins are set according to settings in Configure.usbdmProject
 *
 * @note Only the lower 16-bits of the PCR registers are initialised
 */
void mapAllPins() {
#if true


#endif


#if defined(PCC_PCCn_CGC_MASK)
   PCC->PCC_PORTA = PCC_PCCn_CGC_MASK;
   PCC->PCC_PORTB = PCC_PCCn_CGC_MASK;
   PCC->PCC_PORTC = PCC_PCCn_CGC_MASK;
   PCC->PCC_PORTD = PCC_PCCn_CGC_MASK;
   PCC->PCC_PORTE = PCC_PCCn_CGC_MASK;
#else
   enablePortClocks(PORTA_CLOCK_MASK|PORTB_CLOCK_MASK|PORTC_CLOCK_MASK|PORTD_CLOCK_MASK|PORTE_CLOCK_MASK);
#endif

   PORTA->GPCHR = ForceLockedPins|0x0000UL|PORT_GPCHR_GPWE(0x000CUL);
   PORTA->GPCLR = ForceLockedPins|0x0100UL|PORT_GPCLR_GPWE(0x0010UL);
   PORTA->GPCLR = ForceLockedPins|0x0200UL|PORT_GPCLR_GPWE(0x0006UL);
   PORTA->GPCLR = ForceLockedPins|0x0700UL|PORT_GPCLR_GPWE(0x0009UL);
   PORTB->GPCLR = ForceLockedPins|0x0113UL|PORT_GPCLR_GPWE(0x0003UL);
   PORTB->GPCHR = ForceLockedPins|0x0113UL|PORT_GPCHR_GPWE(0x0003UL);
   PORTB->GPCLR = ForceLockedPins|0x0220UL|PORT_GPCLR_GPWE(0x000CUL);
   PORTC->GPCLR = ForceLockedPins|0x0000UL|PORT_GPCLR_GPWE(0x0080UL);
   PORTC->GPCLR = ForceLockedPins|0x0100UL|PORT_GPCLR_GPWE(0x0020UL);
   PORTC->GPCLR = ForceLockedPins|0x0103UL|PORT_GPCLR_GPWE(0x001EUL);
   PORTC->GPCLR = ForceLockedPins|0x0140UL|PORT_GPCLR_GPWE(0x0041UL);
   PORTD->GPCLR = ForceLockedPins|0x0100UL|PORT_GPCLR_GPWE(0x00F0UL);
   PORTD->GPCLR = ForceLockedPins|0x0113UL|PORT_GPCLR_GPWE(0x000FUL);

   if constexpr (ForceLockoutUnbondedPins) {
      PORTA->GPCLR = PinLock_Locked |0x0000UL|PORT_GPCLR_GPWE(0xFFE0UL); // Lockout unavailable pins
      PORTA->GPCHR = PinLock_Locked |0x0000UL|PORT_GPCHR_GPWE(0xFFF3UL); // Lockout unavailable pins
      PORTB->GPCLR = PinLock_Locked |0x0000UL|PORT_GPCLR_GPWE(0xFFF0UL); // Lockout unavailable pins
      PORTB->GPCHR = PinLock_Locked |0x0000UL|PORT_GPCHR_GPWE(0xFFFCUL); // Lockout unavailable pins
      PORTC->GPCLR = PinLock_Locked |0x0000UL|PORT_GPCLR_GPWE(0xFF00UL); // Lockout unavailable pins
      PORTC->GPCHR = PinLock_Locked |0x0000UL|PORT_GPCHR_GPWE(0xFFFFUL); // Lockout unavailable pins
      PORTD->GPCLR = PinLock_Locked |0x0000UL|PORT_GPCLR_GPWE(0xFF00UL); // Lockout unavailable pins
      PORTD->GPCHR = PinLock_Locked |0x0000UL|PORT_GPCHR_GPWE(0xFFFFUL); // Lockout unavailable pins
      PORTE->GPCLR = PinLock_Locked |0x0000UL|PORT_GPCLR_GPWE(0xFFFFUL); // Lockout unavailable pins
      PORTE->GPCHR = PinLock_Locked |0x0000UL|PORT_GPCHR_GPWE(0xFFFFUL); // Lockout unavailable pins
   }

}
/**
 * End group USBDM_Group
 * @}
 */

} // End namespace USBDM

