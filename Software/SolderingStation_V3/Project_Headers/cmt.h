/**
 * @file     cmt.h (180.ARM_Peripherals/Project_Headers/cmt.h)
 * @brief    Carrier Modulator Transmitter
 *
 * @version  V4.12.1.230
 * @date     13 April 2016
 */

#ifndef HEADER_CMT_H_
#define HEADER_CMT_H_
 /*
 * *****************************
 * *** DO NOT EDIT THIS FILE ***
 * *****************************
 *
 * This file is generated automatically.
 * Any manual changes will be lost.
 */
#include "derivative.h"
#include "hardware.h"

namespace USBDM {

/**
 * @addtogroup CMT_Group CMT, Carrier Modulator Transmitter
 * @brief  Abstraction for Carrier Modulator Transmitter
 * @{
 */

/**
 * Clock Divide Prescaler.
 * Causes the CMT to be clocked at the IF signal frequency, or the IF frequency divided
 * by 2 ,4, or 8.
 */
enum CmtClockDivideBy {
   //   Assuming 8MHz intermediate frequency                 Carrier    Carrier     Modulator
   //                                                        resolution min. period min. period
   CmtClockDivideBy_1 = CMT_MSC_CMTDIV(0),//!< Divide by 1 : 0.125us    0.25us      1.0us
   CmtClockDivideBy_2 = CMT_MSC_CMTDIV(1),//!< Divide by 2 : 0.25us     0.50us      2.0us
   CmtClockDivideBy_4 = CMT_MSC_CMTDIV(2),//!< Divide by 4 : 0.50us     1.00us      4.0us
   CmtClockDivideBy_6 = CMT_MSC_CMTDIV(3),//!< Divide by 6 : 1.00us     2.00us      8.0us
};

/**
 *  Primary Prescaler Divider.
 *  Divides the CMT clock to generate the Intermediate Frequency clock enable to the secondary prescaler.
 *  This divider would usually be chosen to produce a clock of 8MHz from the input Bus clock
 */
enum CmtPrescaler {
   CmtPrescaler_1     = CMT_PPS_PPSDIV(0), //!< Divide by 1
   CmtPrescaler_2     = CMT_PPS_PPSDIV(1), //!< Divide by 2
   CmtPrescaler_3     = CMT_PPS_PPSDIV(2), //!< Divide by 3
   CmtPrescaler_4     = CMT_PPS_PPSDIV(3), //!< Divide by 4
   CmtPrescaler_5     = CMT_PPS_PPSDIV(4), //!< Divide by 5
   CmtPrescaler_6     = CMT_PPS_PPSDIV(5), //!< Divide by 6
   CmtPrescaler_7     = CMT_PPS_PPSDIV(6), //!< Divide by 7
   CmtPrescaler_8     = CMT_PPS_PPSDIV(7), //!< Divide by 8
   CmtPrescaler_9     = CMT_PPS_PPSDIV(8), //!< Divide by 9
   CmtPrescaler_10    = CMT_PPS_PPSDIV(9), //!< Divide by 10
   CmtPrescaler_11    = CMT_PPS_PPSDIV(10),//!< Divide by 11
   CmtPrescaler_12    = CMT_PPS_PPSDIV(11),//!< Divide by 12
   CmtPrescaler_13    = CMT_PPS_PPSDIV(12),//!< Divide by 13
   CmtPrescaler_14    = CMT_PPS_PPSDIV(13),//!< Divide by 14
   CmtPrescaler_15    = CMT_PPS_PPSDIV(14),//!< Divide by 15
   CmtPrescaler_16    = CMT_PPS_PPSDIV(15),//!< Divide by 16
};

/**
 *  Controls main mode of CMT
 */
enum CmtMode {
   CmtMode_Time            = CMT_MSC_MCGEN(1)|CMT_MSC_BASE(0)|CMT_MSC_FSK(0)|CMT_MSC_EXSPC(0), //!< IRO = Primary carrier gated by mark/space
   CmtMode_Baseband        = CMT_MSC_MCGEN(1)|CMT_MSC_BASE(1)|CMT_MSC_FSK(0)|CMT_MSC_EXSPC(0), //!< IRO = Directly controlled by mark/space
   CmtMode_FreqShiftKeying = CMT_MSC_MCGEN(1)|CMT_MSC_BASE(0)|CMT_MSC_FSK(1)|CMT_MSC_EXSPC(0), //!< IRO = Primary/secondary carriers switched by mark/space
   CmtMode_Direct          = CMT_MSC_MCGEN(0)|CMT_MSC_BASE(0)|CMT_MSC_FSK(0)|CMT_MSC_EXSPC(0), //!< IRO = OC[IROL] directly
};


/**
 * Controls Extended space operation
 */
enum CmtExtendedSpace {
   CmtExtendedSpace_Disabled  = CMT_MSC_EXSPC(0), //!< Enables to usual operation
   CmtExtendedSpace_Enabled   = CMT_MSC_EXSPC(1), //!< Forces subsequent cycles to be spaces
};

/**
 *  Enables/Disabled CMT output
 */
enum CmtOutput {
   CmtOutput_Disabled = CMT_OC_IROPEN(0), //!< IRO Pin disabled
   CmtOutput_Enabled  = CMT_OC_IROPEN(1), //!< IRO Pin enabled
};

/**
 * Controls polarity of CMT output
 */
enum CmtPolarity {
   CmtPolarity_ActiveLow  = CMT_OC_CMTPOL(0), //!< Active low
   CmtPolarity_ActiveHigh = CMT_OC_CMTPOL(1), //!< Active high
};

/**
 * Controls CMT Interrupts and DMA
 */
enum CmtInterruptDma {
   CmtInterruptDma_None = CMT_DMA_DMA(0)|CMT_MSC_EOCIE(0), //!< Interrupts and DMA disabled
   CmtInterruptDma_Irq  = CMT_DMA_DMA(0)|CMT_MSC_EOCIE(1), //!< Interrupts enabled
   CmtInterruptDma_Dma  = CMT_DMA_DMA(1)|CMT_MSC_EOCIE(1), //!< DMA enabled
};

/**
 * Type definition for CMT interrupt call back
 */
typedef void (*CMTCallbackFunction)();

/**
 * Template class representing a Carrier Modulator Transmitter.
 *
 * Typical modes of operation:
 * Time mode
 *    Waveform alternates between modulation frequency (mark) and idle (space).
 *    Modulation waveform is controlled by setPrimaryTiming().
 *    Mark/Space time is controlled by setMarkSpaceTiming().
 * Baseband mode
 *    Waveform alternates between high (mark) and idle (space) without use of modulation.
 *    Mark/Space time is controlled by setMarkSpaceTiming().
 * FSK mode
 *    Waveform changes between two modulation frequencies(mark) separated by idle time (space) i.e.
 *    (Primary modulation - idle - Secondary modulation - idle) repeated.
 *    Modulation waveforms are controlled by setPrimaryTiming() and setSecondaryTiming().
 *    Mark/Space time is controlled by setMarkSpaceTiming().
 *    Space time may be set to zero to obtain simple FSK with no idle time.
 *
 * @tparam info      Information class for CMT
 */
template<class Info>
class CmtBase_T {

protected:
   /**
    * Callback to catch unhandled interrupt
    */
   static void unhandledCallback() {
      setAndCheckErrorCode(E_NO_HANDLER);
   }

   /** Callback function for ISR */
   static CMTCallbackFunction sCallback;

public:
   /**
    * Hardware instance pointer.
    *
    * @return Reference to CMT hardware
    */
   static __attribute__((always_inline)) volatile CMT_Type &cmt() { return Info::cmt(); }

   /**
    * Get CMT status.
    * The status is set:\n
    * - When the modulator is not currently active and CMT is enabled to begin the initial CMT transmission.
    * - At the end of each modulation cycle while CMT is enabled.
    *
    * @return Status value
    *
    * @note The status is cleared by either:\n
    * - Calling getStatus() followed by calling getMarkTime(), getSpaceTime() or setMarkSpaceTiming().
    * - A DMA cycle.
    */
   static uint32_t getStatus() {
      return cmt().MSC & CMT_MSC_EOCF_MASK;
   }

   /**
    * IRQ handler
    */
   static void irqHandler() {
      // Call handler
      sCallback();
   }

   /**
    * Set callback function.
    *
    * @param[in] callback Callback function to execute on interrupt.\n
    *                     Use nullptr to remove callback.
    *
    * @note It is expected that the callback will clear the status flag that triggered the interrupt. See getStatus().
    */
   static void setCallback(CMTCallbackFunction callback) {
      static_assert(Info::irqHandlerInstalled, "CMT not configured for interrupts");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      sCallback = callback;
   }

public:
   /**
    * Configures all mapped pins associated with this peripheral
    */
   static void __attribute__((always_inline)) configureAllPins() {
      // Configure pins
      Info::initPCRs();
   }

   /**
    * Basic enable CMT.
    * Includes enabling clock and configuring all pins of mapPinsOnEnable is selected on configuration
    */
   static void enable() {
      if (Info::mapPinsOnEnable) {
         configureAllPins();
      }

      // Enable clock to CMP interface
      Info::enableClock();
   }

   /**
    * Enable with default settings.
    * Includes configuring all pins
    */
   static void defaultConfigure() {
      enable();

      // Initialise hardware
   }

   /**
    * Enable CMT output pin as output.
    * Configures all Pin Control Register (PCR) values
    *
    * @param[in] pcrValue PCR value to use in configuring port (excluding MUX value). See pcrValue()
    */
   static void setOutput(PcrValue pcrValue=Info::defaultPcrValue) {
      using Pcr = PcrTable_T<Info, 0>;

      // Enable and map pin to CMP_OUT
      Pcr::setPCR((pcrValue&~PORT_PCR_MUX_MASK)|(Info::info[0].pcrValue&PORT_PCR_MUX_MASK));
   }

   /**
    * Enable CMT output pin as output.
    * Configures all Pin Control Register (PCR) values
    *
    * @param[in] pinDriveStrength One of PinDriveStrength_Low, PinDriveStrength_High
    * @param[in] pinDriveMode     One of PinDriveMode_PushPull, PinDriveMode_OpenDrain (defaults to PinPushPull)
    * @param[in] pinSlewRate      One of PinSlewRate_Slow, PinSlewRate_Fast (defaults to PinSlewRate_Fast)
    */
   static void setOutput(
         PinDriveStrength  pinDriveStrength,
         PinDriveMode      pinDriveMode      = PinDriveMode_PushPull,
         PinSlewRate       pinSlewRate       = PinSlewRate_Fast
         ) {
      setOutput(pinDriveStrength|pinDriveMode|pinSlewRate);
   }

   /**
    * Select Primary Prescaler Divider.
    * This divider would usually be chosen to produce a clock of 8MHz from the input Bus clock
    *
    * @param[in] cmtPrescaler
    */
   static void setPrescaler(CmtPrescaler cmtPrescaler) {
      cmt().PPS = cmtPrescaler;
   }

   /**
    * Base configuration of CMT.
    * The CMT clock is configured for a nominal 8MHz frequency
    *
    * @param[in] cmtMode             Basic mode
    * @param[in] cmtClockDivideBy    Divider for carrier (from nominal 8MHz)
    */
   static void configure(CmtMode cmtMode, CmtClockDivideBy cmtClockDivideBy=CmtClockDivideBy_1) {
      enable();
      setPrescaler((CmtPrescaler)((SystemBusClock/8000000)-1));
      cmt().MSC = cmtMode|cmtClockDivideBy;
   }

   /**
    * Output control
    *
    * @param[in] cmtOutput     Enable/disable output
    * @param[in] cmtPolarity   Polarity of output
    */
   static void outputControl(CmtOutput cmtOutput, CmtPolarity cmtPolarity=CmtPolarity_ActiveHigh) {
      cmt().OC = cmtOutput|cmtPolarity;
   }

   /**
    * Base mode of CMT
    *
    * @param[in] cmtMode             Basic mode
    */
   static void setMode(CmtMode cmtMode) {
      cmt().MSC = (cmt().MSC&~(CMT_MSC_MCGEN(1)|CMT_MSC_BASE(1)|CMT_MSC_FSK(1)))|cmtMode;
   }

   /**
    * Controls Extended space operation
    *
    * @param[in] cmtExtendedSpace Allows Forcing of subsequent cycles to be spaces
    */
   static void setExtendedSpace(CmtExtendedSpace cmtExtendedSpace=CmtExtendedSpace_Enabled) {
      cmt().MSC = (cmt().MSC&~CMT_MSC_EXSPC(1))|cmtExtendedSpace;
   }

   /**
    * Get high time for primary carrier frequency
    *
    * @return High time in clock cycles (usually 125ns)
    */
   static uint8_t getPrimaryHigh() {
      return cmt().CGH1;
   }

   /**
    * Get low time for primary carrier frequency
    *
    * @return Low time in clock cycles (usually 125ns)
    */
   static uint8_t getPrimaryLow() {
      return cmt().CGL1;
   }

   /**
    * Set high/low times for primary carrier frequency
    *
    * @param[in] high    High time in clock cycles (usually 125ns). Must be >0.
    * @param[in] low     Low time in clock cycles (usually 125ns). Must be >0.
    */
   static void setPrimaryTiming(uint8_t high, uint8_t low) {
      usbdm_assert((high>0)&&(low>0), "High/low values must be non-zero");
      cmt().CGH1 = high;
      cmt().CGL1 = low;
   }

   /**
    * Get high time for secondary carrier frequency
    *
    * @return High time in clock cycles (usually 125ns)
    */
   static uint8_t getSecondaryHigh() {
      return cmt().CGH2;
   }

   /**
    * Get low time for secondary carrier frequency
    *
    * @return Low time in clock cycles (usually 125ns)
    */
   static uint8_t getSecondaryLow() {
      return cmt().CGL2;
   }

   /**
    * Set high/low times for secondary carrier frequency (for FSK)
    *
    * @param[in] high    High time in carrier cycles (usually 125ns). Must be >0.
    * @param[in] low     Low time in carrier cycles (usually 125ns). Must be >0.
    */
   static void setSecondaryTiming(uint8_t high, uint8_t low) {
      usbdm_assert((high>0)&&(low>0), "High/low values must be non-zero");
      cmt().CGH2 = high;
      cmt().CGL2 = low;
   }

   /**
    * Get mark time for modulation
    *
    * @return Mark time in modulation cycles
    */
   static uint16_t getMarkTime() {
      return (cmt().CMD1<<8)|cmt().CMD2;
   }

   /**
    * Get space time for modulation
    *
    * @return Space time in modulation cycles
    */
   static uint16_t getSpaceTime() {
      return (cmt().CMD3<<8)|cmt().CMD4;
   }

   /**
    * Set Mark values for modulation
    *
    * @param[in] mark    Mark time in carrier frequency cycles
    */
   static void setMarkTiming(uint16_t mark) {
      cmt().CMD1 = (uint8_t)(mark>>8);
      cmt().CMD2 = (uint8_t)(mark);
    }

   /**
    * Set Space values for modulation
    *
    * @param[in] space   Space time in carrier frequency cycles
    */
   static void setSpaceTiming(uint16_t space) {
      cmt().CMD3 = (uint8_t)(space>>8);
      cmt().CMD4 = (uint8_t)(space);
    }

   /**
    * Set Mark/Space values for modulation
    *
    * @param[in] mark    Mark time in carrier frequency cycles
    * @param[in] space   Space time in carrier frequency cycles
    */
   static void setMarkSpaceTiming(uint16_t mark, uint16_t space) {
      cmt().CMD1 = (uint8_t)(mark>>8);
      cmt().CMD2 = (uint8_t)(mark);
      cmt().CMD3 = (uint8_t)(space>>8);
      cmt().CMD4 = (uint8_t)(space);
    }

   /**
    * Disable CMT
    */
   static void disable() {
      cmt().MSC = 0;
      disableNvicInterrupts();
      Info::disableClock();
   }

   /**
    * Enable interrupts in NVIC
    */
   static void enableNvicInterrupts() {
      NVIC_EnableIRQ(Info::irqNums[0]);
   }

   /**
    * Enable and set priority of interrupts in NVIC
    * Any pending NVIC interrupts are first cleared.
    *
    * @param[in]  nvicPriority  Interrupt priority
    */
   static void enableNvicInterrupts(NvicPriority nvicPriority) {
      enableNvicInterrupt(Info::irqNums[0], nvicPriority);
   }

   /**
    * Disable interrupts in NVIC
    */
   static void disableNvicInterrupts() {
      NVIC_DisableIRQ(Info::irqNums[0]);
   }

   /**
    * Enable Interrupt or DMA operation
    *
    * @param[in] cmtInterruptDma Selects Interrupts and DMA operation
    */
   static void enableInterruptDma(CmtInterruptDma cmtInterruptDma) {
      switch (cmtInterruptDma) {
         case CmtInterruptDma_None:
            cmt().MSC &= ~CMT_MSC_EOCIE_MASK;
            cmt().DMA &= ~CMT_DMA_DMA_MASK;
            break;
         case CmtInterruptDma_Irq:
            cmt().DMA &= ~CMT_DMA_DMA_MASK;
            cmt().MSC |= CMT_MSC_EOCIE_MASK;
            break;
         case CmtInterruptDma_Dma:
            cmt().DMA |= CMT_DMA_DMA_MASK;
            cmt().MSC |= CMT_MSC_EOCIE_MASK;
            break;
      }
   }
};

template<class Info> CMTCallbackFunction CmtBase_T<Info>::sCallback = CmtBase_T<Info>::unhandledCallback;

#if defined(USBDM_CMT_IS_DEFINED)
class Cmt : public CmtBase_T<CmtInfo> {};
#endif

#if defined(USBDM_CMT1_IS_DEFINED)
class Cmt1 : public CmtBase_T<Cmt1Info> {};
#endif

#if defined(USBDM_CMT2_IS_DEFINED)
class Cmt2 : public CmtBase_T<Cmt2Info> {};
#endif

/**
 * End CMT_Group
 * @}
 */
} // End namespace USBDM

#endif /* HEADER_CMT_H_ */
