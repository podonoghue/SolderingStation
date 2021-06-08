/**
 * @file     lptmr.h (180.ARM_Peripherals/Project_Headers/lptmr.h)
 * @brief    Low power timer interface
 *
 * @version  V4.12.1.80
 * @date     13 April 2016
 */
#ifndef HEADER_LPTMR_H
#define HEADER_LPTMR_H
 /*
 * *****************************
 * *** DO NOT EDIT THIS FILE ***
 * *****************************
 *
 * This file is generated automatically.
 * Any manual changes will be lost.
 */
#include "derivative.h"
#include "system.h"
#include "hardware.h"
#include "cmath"

namespace USBDM {

/**
 * @addtogroup LPTMR_Group LPTMR, Low Power Timer
 * @brief Abstraction for Low Power Timer
 * @{
 */

/**
 * Type definition for LPTMR interrupt call back
 */
typedef void (*LPTMRCallbackFunction)(void);

#ifdef PCC_PCC_LPTMR0_CGC_MASK
/**
 * Select the LPTMR clock source which determines count speed or glitch filtering
 */
enum LptmrClockSel {
   LptmrClockSel_SircDiv2Clk  = LPTMR_PSR_PCS(0), //!< Slow Internal Reference Div 2 Clock (SIRCDIV2_CLK)
   LptmrClockSel_Lpo1Kclk     = LPTMR_PSR_PCS(1), //!< Low power oscillator 1kHz (LPO1K_CLK)
   LptmrClockSel_Rtcclk       = LPTMR_PSR_PCS(2), //!< 32kHz Clock Source (RTC_CLK)
   LptmrClockSel_PccLptmrClk  = LPTMR_PSR_PCS(3), //!< Clock from PCC_LPTMRx multiplexor (PCC)
   LptmrClockSel_Default      = LptmrClockSel_Lpo1Kclk,
};
#else
/**
 * Select the LPTMR clock source which determines count speed or glitch filtering
 */
enum LptmrClockSel {
   LptmrClockSel_Mcgirclk = LPTMR_PSR_PCS(0), //!< MCG Internal Reference Clock (MCGIRCLK)
   LptmrClockSel_Lpoclk   = LPTMR_PSR_PCS(1), //!< Low power oscillator (LPO - 1kHz)
   LptmrClockSel_Erclk32  = LPTMR_PSR_PCS(2), //!< 32kHz Clock Source (ERCLK32)
   LptmrClockSel_Oscerclk = LPTMR_PSR_PCS(3), //!< Oscillator External Reference Clock (OSCERCLK)
   LptmrClockSel_Default  = LptmrClockSel_Lpoclk,
};
#endif

/**
 * Select the LPTMR clock pre-scale which affect counter speed/glitch filtering
 */
enum LptmrPrescale {
   LptmrPrescale_Bypass = LPTMR_PSR_PBYP(1),       //!< Divide is bypassed (no divider/glitch filtering)
   LptmrPrescale_2      = LPTMR_PSR_PRESCALE(0),   //!< Divide by 2
   LptmrPrescale_4      = LPTMR_PSR_PRESCALE(1),   //!< Divide by 4
   LptmrPrescale_8      = LPTMR_PSR_PRESCALE(2),   //!< Divide by 8
   LptmrPrescale_16     = LPTMR_PSR_PRESCALE(3),   //!< Divide by 16
   LptmrPrescale_32     = LPTMR_PSR_PRESCALE(4),   //!< Divide by 32
   LptmrPrescale_64     = LPTMR_PSR_PRESCALE(5),   //!< Divide by 64
   LptmrPrescale_128    = LPTMR_PSR_PRESCALE(6),   //!< Divide by 128
   LptmrPrescale_256    = LPTMR_PSR_PRESCALE(7),   //!< Divide by 256
   LptmrPrescale_512    = LPTMR_PSR_PRESCALE(8),   //!< Divide by 512
   LptmrPrescale_1024   = LPTMR_PSR_PRESCALE(9),   //!< Divide by 1024
   LptmrPrescale_2048   = LPTMR_PSR_PRESCALE(10),  //!< Divide by 2048
   LptmrPrescale_4096   = LPTMR_PSR_PRESCALE(11),  //!< Divide by 4096
   LptmrPrescale_8192   = LPTMR_PSR_PRESCALE(12),  //!< Divide by 8192
   LptmrPrescale_16384  = LPTMR_PSR_PRESCALE(13),  //!< Divide by 16384
   LptmrPrescale_32768  = LPTMR_PSR_PRESCALE(14),  //!< Divide by 32768
   LptmrPrescale_65536  = LPTMR_PSR_PRESCALE(15),  //!< Divide by 65536
};

/**
 * Select the LPTMR clock input pin
 */
enum LptmrPinSel {
   LptmrPinSel_Cmp0 = LPTMR_CSR_TPS(0), //!< CMP pin
   LptmrPinSel_Alt1 = LPTMR_CSR_TPS(1), //!< LPTMR_ALT1 pin
   LptmrPinSel_Alt2 = LPTMR_CSR_TPS(2), //!< LPTMR_ALT2 pin
};

/**
 * Select the LPTMR Pulse Mode input pin polarity
 */
enum LptmrPulseEdge {
   LptmrPulse_RisingEdge  = LPTMR_CSR_TPP(0), //!< LPTMR time counting mode
   LptmrPulse_FallingEdge = LPTMR_CSR_TPP(1), //!< LPTMR pulse counting mode
};

/**
 * Select when the LPTMR counter resets to zero
 */
enum LptmrResetOn {
   LptmrResetOn_Compare  = LPTMR_CSR_TFC(0), //!< LPTMR counter is reset whenever TCF is set.
   LptmrResetOn_Overflow = LPTMR_CSR_TFC(1), //!< LPTMR counter is reset on overflow.
};

/**
 * Select LPTMR mode
 */
enum LptmrMode {
   LptmrMode_Time          = LPTMR_CSR_TMS(0), //!< LPTMR operates in time mode
   LptmrMode_PulseCounting = LPTMR_CSR_TMS(1), //!< LPTMR operates in pulse counting mode
};

/**
 * Enable/Disable LPTMR interrupts
 */
enum LptmrInterrupt {
   LptmrInterrupt_Disabled = LPTMR_CSR_TIE(0), //!< Disable LPTMR interrupts
   LptmrInterrupt_Enabled  = LPTMR_CSR_TIE(1), //!< Enable LPTMR interrupts
};

/**
 * @brief Template class representing a Low Power Timer
 */
template<class Info>
class LptmrBase_T {

protected:
   /** Minimum resolution required when setting interval */
   static constexpr int MINIMUM_RESOLUTION = 100;

   /** Callback function for ISR */
   static LPTMRCallbackFunction sCallback;

   /** Hardware instance */
   static __attribute__((always_inline)) volatile LPTMR_Type &lptmr()     { return Info::lptmr(); }

   /** Callback to catch unhandled interrupt */
   static void unhandledCallback() {
      setAndCheckErrorCode(E_NO_HANDLER);
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
    * Enables the LPTMR clock and configures the pins
    */
   static void enable() {
      configureAllPins();

      // Enable clock
      Info::enableClock();
      __DMB();
   }
   
   /**
    * Set LPTMR to pulse counting mode.
    * Provides selection of input pin, edge selection and reset mode.\n
    * The timer is enabled and pins configured.
    *
    * @param[in] lptmrPinSel     Input pin for Pulse Counting mode
    * @param[in] lptmrPulseEdge  Edge for pulse counting (default = rising-edge)
    * @param[in] lptmrResetOn    Selects when the LPTMR counter resets to zero (default = on overflow)
    * @param[in] lptmrInterrupt  Enable/disable interrupts
    */
   static void configurePulseCountingMode(
         LptmrPinSel       lptmrPinSel,
         LptmrPulseEdge    lptmrPulseEdge = LptmrPulse_RisingEdge,
         LptmrResetOn      lptmrResetOn   = LptmrResetOn_Overflow,
         LptmrInterrupt    lptmrInterrupt = LptmrInterrupt_Disabled) {

      enable();
      // Change settings with timer disabled
      lptmr().CSR = LptmrMode_PulseCounting|lptmrPinSel|lptmrPulseEdge|lptmrResetOn|lptmrInterrupt;
      // Enable timer
      lptmr().CSR = LptmrMode_PulseCounting|lptmrPinSel|lptmrPulseEdge|lptmrResetOn|lptmrInterrupt|LPTMR_CSR_TEN_MASK;
   }

   /**
    * Set LPTMR to time counting mode.
    * The timer is enabled and pins configured.
    *
    * @param[in] lptmrResetOn    Selects when the LPTMR counter resets to zero
    * @param[in] lptmrInterrupt  Enable/disable interrupts
    * @param[in] lptmrClockSel   Clock source selection
    * @param[in] lptmrPrescale   Clock divider
    */
   static void configureTimeCountingMode(
         LptmrResetOn      lptmrResetOn   = LptmrResetOn_Compare,
         LptmrInterrupt    lptmrInterrupt = LptmrInterrupt_Disabled,
         LptmrClockSel     lptmrClockSel  = LptmrClockSel_Default,
         LptmrPrescale     lptmrPrescale  = LptmrPrescale_Bypass) {
      enable();
      // Change settings with timer disabled
      lptmr().CSR = LptmrMode_Time|lptmrResetOn|lptmrInterrupt;
      // Set clock source and prescaler
      lptmr().PSR = lptmrClockSel|lptmrPrescale;
      // Set dummy timer value to avoid immediate interrupts
      lptmr().CMR = (uint32_t)-1;
      // Enable timer and clear interrupt flag
      lptmr().CSR = LptmrMode_Time|lptmrResetOn|lptmrInterrupt|LPTMR_CSR_TEN_MASK|LPTMR_CSR_TCF_MASK;
   }

   /**
    * Restarts the counter\n
    * Mostly for debug.
    *
    */
   static void restart() {
      uint32_t csr = lptmr().CSR;
      lptmr().CSR   = 0;
      lptmr().CSR   = csr|LPTMR_CSR_TCF_MASK;
   }

   /**
    * Set LPTMR clock source and prescaler.
    * These settings are used for the clock prescaler in timer mode and glitch filter in pulse-counting mode.
    *
    * @param[in] lptmrClockSel   Clock source selection
    * @param[in] lptmrPrescale   Clock divider
    */
   static void setClock(
         LptmrClockSel lptmrClockSel,
         LptmrPrescale lptmrPrescale   = LptmrPrescale_Bypass) {

      uint32_t csr = lptmr().CSR;
      lptmr().CSR   = 0;
      lptmr().PSR   = lptmrClockSel|lptmrPrescale;
      lptmr().CSR   = csr;
   }

   /**
    * Enable/disable interrupts in LPTMR
    *
    * @param[in]  enable true to enable, false to disable
    */
   static void enableInterrupts(bool enable=true) {
      if (enable) {
         lptmr().CSR |= LPTMR_CSR_TIE_MASK;
      }
      else {
         lptmr().CSR &= ~LPTMR_CSR_TIE_MASK;
      }
   }

   /**
    * Clear interrupt flag
    */
   static void clearInterruptFlag() {
      lptmr().CSR |= LPTMR_CSR_TCF_MASK;
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
   static void enableNvicInterrupts(uint32_t nvicPriority) {
      enableNvicInterrupt(Info::irqNums[0], nvicPriority);
   }

   /**
    * Disable interrupts in NVIC
    */
   static void disableNvicInterrupts() {
      NVIC_DisableIRQ(Info::irqNums[0]);
   }

   /**
    * Set callback for ISR and enable NVIC interrupts.
    *
    *   @param[in]  callback Callback function to be executed on interrupt\n
    *                        Use nullptr to remove callback.
    */
   static void setCallback(LPTMRCallbackFunction callback) {
      static_assert(Info::irqHandlerInstalled, "LPTMR not configure for interrupts");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      sCallback = callback;
   }

   /**
    * PIT interrupt handler. \n
    * Calls PIT0 callback
    */
   static void irqHandler() {
      // Clear interrupt flag
      lptmr().CSR |= LPTMR_CSR_TCF_MASK;

      sCallback();
   }

   /**
    * Enable LPTMR with default configuration.
    *
    * Includes enabling clock and any pins used.\n
    * Sets LPTMR to default configuration
    */
   static void defaultConfigure() {
      enable();
      // Disable timer
      lptmr().CSR  = Info::csr;
      // PCS 0,1,2,3 => MCGIRCLK, LPO, ERCLK32K, OSCERCLK
      lptmr().PSR  = Info::psr;
      // Period/Compare value
      lptmr().CMR  = Info::cmr;
      // Enable timer
      lptmr().CSR |= LPTMR_CSR_TEN_MASK;

      if (Info::csr & LPTMR_CSR_TIE_MASK) {
         // Enable timer interrupts
         NVIC_EnableIRQ(Info::irqNums[0]);

         // Set priority level
         NVIC_SetPriority(Info::irqNums[0], Info::irqLevel);
      }
   }
   /**
    *   Disable the LPTMR
    */
   static void disable(void) {
      // Disable timer
      lptmr().CSR = 0;
      NVIC_DisableIRQ(Info::irqNums[0]);
      Info::disableClock();
   }

   /**
    * Converts a number in ticks to time in microseconds.
    *
    * @param[in]  ticks Time in ticks
    *
    * @return Time in microseconds
    *
    * @note Assumes prescale has been chosen appropriately.
    * @note Rudimentary range checking only. Sets error code.
    */
   static uint32_t convertTicksToMicroseconds(unsigned ticks) {
      uint32_t tickRate = Info::getClockFrequency();
      uint64_t rv       = (((uint64_t)ticks)*1000000)/tickRate;

#ifdef DEBUG_BUILD
      if (rv > UINT_MAX) {
         // Attempt to set too long a period
         setErrorCode(E_TOO_LARGE);
      }
      if (rv == 0) {
         // Attempt to set too short a period
         setErrorCode(E_TOO_SMALL);
      }
#endif
      return rv;
   }

   /**
    * Converts a number in ticks to time in milliseconds.
    *
    * @param[in]  ticks Time in ticks
    *
    * @return Time in milliseconds
    *
    * @note Assumes prescale has been chosen appropriately.
    * @note Rudimentary range checking only. Sets error code.
    */
   static unsigned convertTicksToMilliseconds(unsigned ticks) {
      uint32_t tickRate = Info::getClockFrequency();
      uint64_t rv       = (((uint64_t)ticks)*1000)/tickRate;

#ifdef DEBUG_BUILD
      if (rv > UINT_MAX) {
         // Attempt to set too long a period
         setErrorCode(E_TOO_LARGE);
      }
      if (rv == 0) {
         // Attempt to set too short a period
         setErrorCode(E_TOO_SMALL);
      }
#endif
      return rv;
   }

   /**
    * Converts a number in ticks to time in seconds.
    *
    * @param[in]  ticks Time in ticks
    *
    * @return Time in seconds (as float)
    *
    * @note Assumes prescale has been chosen appropriately.
    * @note Rudimentary range checking only. Sets error code.
    */
   static float convertTicksToSeconds(unsigned ticks) {
      uint32_t tickRate = Info::getClockFrequency();
      return ((float)ticks)/tickRate;
   }

   /**
    * Converts a time in microseconds to number of ticks.
    *
    * @param[in]  time Time in microseconds
    *
    * @return Time in ticks
    *
    * @note Assumes prescale has been chosen appropriately.
    * @note Rudimentary range checking only. Sets error code.
    */
   static uint32_t convertMicrosecondsToTicks(int time) {

      // Calculate period
      uint32_t tickRate = Info::getClockFrequency();
      uint64_t rv       = ((uint64_t)time*tickRate)/1000000;

#ifdef DEBUG_BUILD
      if (rv > 0xFFFFUL) {
         // Attempt to set too long a period
         setErrorCode(E_TOO_LARGE);
      }
      if (rv == 0) {
         // Attempt to set too short a period
         setErrorCode(E_TOO_SMALL);
      }
#endif
      return rv;
   }
   /**
    * Converts a time in milliseconds to number of ticks.
    *
    * @param[in]  time Time in milliseconds
    *
    * @return Time in ticks
    *
    * @note Assumes prescale has been chosen appropriately.
    * @note Rudimentary range checking only. Sets error code.
    */
   static uint32_t convertMillisecondsToTicks(int time) {

      // Calculate period
      uint32_t tickRate = Info::getClockFrequency();
      uint64_t rv       = ((uint64_t)time*tickRate)/1000;

#ifdef DEBUG_BUILD
      if (rv > 0xFFFFUL) {
         // Attempt to set too long a period
         setErrorCode(E_TOO_LARGE);
      }
      if (rv == 0) {
         // Attempt to set too short a period
         setErrorCode(E_TOO_SMALL);
      }
#endif
      return rv;
   }

   /**
    * Converts a time in seconds to number of ticks
    *
    * @param[in]  time Time in seconds (float!)
    *
    * @return Time in ticks
    *
    * @note Uses floating point
    * @note Rudimentary range checking only. Sets error code.
    */
   static uint32_t convertSecondsToTicks(float time) {

      // Calculate period
      float    tickRate = Info::getClockFrequencyF();
      uint64_t rv       = (time*tickRate);

#ifdef DEBUG_BUILD
      if (rv > 0xFFFFUL) {
         // Attempt to set too long a period
         setErrorCode(E_TOO_LARGE);
      }
      if (rv == 0) {
         // Attempt to set too long a period
         setErrorCode(E_TOO_SMALL);
      }
#endif
      return rv;
   }

   /**
    * Set period of timer.
    *
    * @param[in]  period Period in seconds as a float
    *
    * @note Will enable and adjust the pre-scaler to appropriate value.\n
    *       The clock source should be selected by setClock() before using this function.
    *
    * @return E_NO_ERROR      => Success
    * @return E_ILLEGAL_PARAM => Failed to find suitable values for PBYP & PRESCALE
    */
   static ErrorCode setPeriod(float period) {
      // Disable LPTMR before prescale change
      uint32_t csr = lptmr().CSR;
      lptmr().CSR = 0;

      float    inputClock = Info::getInputClockFrequency();
      int      prescaleFactor=1;
      uint32_t prescalerValue=0;
      while (prescalerValue<=16) {
         float    clockFrequency = inputClock/prescaleFactor;
         uint32_t mod   = rintf(period*clockFrequency);
         if (mod < MINIMUM_RESOLUTION) {
            // Too short a period for reasonable resolution
            return setAndCheckErrorCode(E_TOO_SMALL);
         }
         if (mod <= 65535) {
            __DSB();
            lptmr().CMR  = mod;
            lptmr().PSR  = (lptmr().PSR & ~(LPTMR_PSR_PRESCALE_MASK|LPTMR_PSR_PBYP_MASK))|LPTMR_PSR_PRESCALE(prescalerValue-1)|LPTMR_PSR_PBYP(prescalerValue==0);
            lptmr().CSR  = csr;
            return E_NO_ERROR;
         }
         prescalerValue++;
         prescaleFactor <<= 1;
      }
      // Too long a period
      return setAndCheckErrorCode(E_TOO_LARGE);
   }

   /**
    * Set glitch filter interval.
    * This adjusts the clock prescaler so that the filter interval is at least the given value.
    *
    * @param[in]  interval Interval in seconds as a float
    *
    * @note Will enable and adjust the pre-scaler to appropriate value.\n
    *       The clock source should be selected by setClock() before using this function.
    *
    * @return E_NO_ERROR      => Success
    * @return E_ILLEGAL_PARAM => Failed to find suitable values for PBYP & PRESCALE
    */
   static ErrorCode setFilterInterval(float interval) {
      long     inputClock = Info::getInputClockFrequency();
      int      prescaleFactor=1;
      uint32_t prescalerValue=0;
      while (prescalerValue<=16) {
         if ((interval*prescaleFactor) < inputClock) {
            // Disable LPTMR before prescale change
            uint32_t csr = lptmr().CSR;
            lptmr().CSR = 0;
            __DSB();
            lptmr().PSR  = (lptmr().PSR & ~(LPTMR_PSR_PRESCALE_MASK|LPTMR_PSR_PBYP_MASK))|LPTMR_PSR_PRESCALE(prescalerValue-1)|LPTMR_PSR_PBYP(prescalerValue==0);
            lptmr().CSR  = csr;
            return E_NO_ERROR;
         }
         prescalerValue++;
         prescaleFactor <<= 1;
      }
      // Too long a period
      return setAndCheckErrorCode(E_TOO_LARGE);
   }

   /**
    * Get timer counter value
    *
    * @return Timer value in ticks.
    */
   static uint32_t getCounterValue() {
      // It is necessary to write to the CNR to capture current value
      lptmr().CNR = 0;
      return lptmr().CNR;
   }
};

template<class Info> LPTMRCallbackFunction LptmrBase_T<Info>::sCallback = LptmrBase_T<Info>::unhandledCallback;

#ifdef USBDM_LPTMR0_IS_DEFINED
/**
 * @brief Class representing LPTMR0
 *
 * <b>Example</b>
 * @code
 *
 * // LPTMR callback
 * void flash(void) {
 *    RED_LED::toggle();
 * }
 *
 * ...
 * // Configure LPTMR in time counting mode
 * Lptmr0::configureTimeCountingMode(
 *      LptmrResetOn_Compare,
 *      LptmrInterrupt_Enabled,
 *      LptmrClockSel_Lpoclk);
 *
 * // Set period of timer event
 * Lptmr0::setPeriod(5*seconds);
 *
 * // Set call-back
 * Lptmr0::setCallback(flash);
 * @endcode
 */
using Lptmr0 = LptmrBase_T<Lptmr0Info>;
#endif

#ifdef USBDM_LPTMR1_IS_DEFINED
/**
 * @brief Class representing LPTMR1
 *
 * <b>Example</b>
 * @code
 *
 * // LPTMR callback
 * void flash(void) {
 *    RED_LED::toggle();
 * }
 *
 * ...
 *
 * // Configure LPTMR in time counting mode
 * Lptmr1::configureTimeCountingMode(
 *      LptmrResetOn_Compare,
 *      LptmrInterrupt_Enabled,
 *      LptmrClockSel_Lpoclk);
 *
 * // Set period of timer event
 * Lptmr1::setPeriod(5*seconds);
 *
 * // Set call-back
 * Lptmr1::setCallback(flash);
 * @endcode
 */
using Lptmr1 = LptmrBase_T<Lptmr1Info>;
#endif

/**
 * End LPTMR_Group
 * @}
 */

} // End namespace USBDM

#endif /* HEADER_LPTMR_H */
