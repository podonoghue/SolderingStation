/**
 * @file     pit.h (180.ARM_Peripherals/Project_Headers/pit-MK.h)
 *
 * @brief    Programmable Interrupt Timer interface
 *
 * @version  V4.12.1.80
 * @date     13 April 2016
 */

#ifndef INCLUDE_USBDM_PIT_H_
#define INCLUDE_USBDM_PIT_H_
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

namespace USBDM {

enum DmaChannelNum : unsigned;

/**
 * @addtogroup PIT_Group PIT, Programmable Interrupt Timer
 * @brief Abstraction for Programmable Interrupt Timer
 * @{
 */
/**
 * Type definition for PIT interrupt call back
 */
typedef void (*PitCallbackFunction)(void);

/**
 * Control PIT operation in debug mode (suspended for debugging)
 */
enum PitDebugMode {
   PitDebugMode_Run  = PIT_MCR_FRZ(0),  //!< PIT continues to run in debug mode
   PitDebugMode_Stop = PIT_MCR_FRZ(1),  //!< PIT stops in debug mode
};

/**
 * Enable the PIT interrupts
 */
enum PitChannelIrq {
   PitChannelIrq_Disabled  = PIT_TCTRL_TIE(0),  //!< PIT channel interrupt disabled
   PitChannelIrq_Enabled   = PIT_TCTRL_TIE(1),  //!< PIT channel interrupt disabled
};

/**
 * Enable the PIT channel
 */
enum PitChannelEnable {
   PitChannelEnable_Disabled  = PIT_TCTRL_TEN(0),  //!< PIT channel disabled
   PitChannelEnable_Enabled   = PIT_TCTRL_TEN(1),  //!< PIT channel enabled
};

/**
 * Channel numbers.
 */
enum PitChannelNum : unsigned {
   PitChannelNum_0,      //!< Channel  0
   PitChannelNum_1,      //!< Channel  1
   PitChannelNum_2,      //!< Channel  2
   PitChannelNum_3,      //!< Channel  3

   PitChannelNum_None = (1<<7),  //!< Used to indicate failed channel allocation
};

/**
 * Calculate a PIT channel number using an offset from an existing number
 *
 * @param pitChannelNum Base channel to use
 * @param offset  Offset from base channel
 *
 * @return  PIT channel number calculated from channel+offset
 */
constexpr PitChannelNum inline operator+(PitChannelNum pitChannelNum, unsigned offset) {
   return (PitChannelNum)((unsigned)pitChannelNum + offset);
}

/**
 * Calculate a PIT channel number using an offset from an existing number
 *
 * @param pitChannelNum Base channel to use
 * @param offset  Offset from base channel
 *
 * @return  PIT channel number calculated from channel+offset
 */
constexpr PitChannelNum inline operator+(PitChannelNum pitChannelNum, int offset) {
   return pitChannelNum + (unsigned)offset;
}

/**
 * @brief Class representing a Programmable Interrupt  Timer
 *
 * <b>Example</b>
 * @code
 *
 * @endcode
 */
template<class Info>
class PitBase_T {

private:
   /**
    * This class is not intended to be instantiated
    */
   PitBase_T() = delete;
   PitBase_T(const PitBase_T&) = delete;
   PitBase_T(PitBase_T&&) = delete;

   /** Bit-mask of allocated channels */
   static uint32_t allocatedChannels;

protected:
   /** Default TCTRL value for timer channel */
   static constexpr uint32_t PIT_TCTRL_DEFAULT_VALUE = (PIT_TCTRL_TEN_MASK);

   /** Callback functions for ISRs */
   static PitCallbackFunction sCallbacks[Info::NumChannels];

   /** Bitmask used to indicate a channel call-back is one-shot */
   static uint8_t clearOnEvent;

   /** Callback to catch unhandled interrupt */
   static void unhandledCallback() {
      setAndCheckErrorCode(E_NO_HANDLER);
   }

public:
   /**
    * Allocate PIT channel.
    *
    * @return PitChannelNum_None - No suitable channel available.  Error code set.
    * @return Channel number     - Number of allocated channel
    */
   static PitChannelNum allocateChannel() {
      CriticalSection cs;
      unsigned channelNum = __builtin_ffs(allocatedChannels);
      if ((channelNum == 0)||(--channelNum>=Info::NumChannels)) {
         setErrorCode(E_NO_RESOURCE);
         return PitChannelNum_None;
      }
      allocatedChannels &= ~(1<<channelNum);
      return (PitChannelNum) channelNum;
   }

   /**
    * Allocate PIT channel associated with DMA channel.
    * This is a channel that may be used to throttle the associated DMA channel.
    *
    * @param dmaChannelNum DMA channel being used.
    *
    * @return PitChannelNum_None - No suitable channel available.  Error code set.
    * @return Channel number     - Number of allocated channel
    */
   static PitChannelNum allocateDmaAssociatedChannel(DmaChannelNum dmaChannelNum) {
      const uint32_t channelMask = (1<<dmaChannelNum);
      usbdm_assert(dmaChannelNum<Info::NumChannels, "No PIT channel associated with DMA channel");
      CriticalSection cs;
      usbdm_assert((allocatedChannels & channelMask) != 0, "PIT channel already allocated");
      if ((allocatedChannels & channelMask) == 0) {
         setErrorCode(E_NO_RESOURCE);
         return PitChannelNum_None;
      }
      allocatedChannels &= ~channelMask;
      return (PitChannelNum) dmaChannelNum;
   }

   /**
    * Free PIT channel.
    * Disables the channel.
    *
    * @param pitChannelNum Channel to release
    */
   static void freeChannel(PitChannelNum pitChannelNum) {
      if (pitChannelNum == PitChannelNum_None) {
         return;
      }
      const uint32_t channelMask = (1<<pitChannelNum);
      usbdm_assert(pitChannelNum<Info::NumChannels, "Illegal PIT channel");
      usbdm_assert((allocatedChannels & channelMask) == 0, "Freeing unallocated PIT channel");

      disableChannel(pitChannelNum);
      CriticalSection cs;
      allocatedChannels |= channelMask;
   }

   /**
    * Enable/disable channel interrupts
    *
    * @param[in]  pitChannelNum Channel being modified
    * @param[in]  enable  True => enable, False => disable
    */
   static void enableInterrupts(PitChannelNum pitChannelNum, bool enable=true) {
      if (enable) {
         pit().CHANNEL[pitChannelNum].TCTRL |= PIT_TCTRL_TIE_MASK;
      }
      else {
         pit().CHANNEL[pitChannelNum].TCTRL &= ~PIT_TCTRL_TIE_MASK;
      }
   }


   /**
    * Set interrupt callback
    *
    *  @param[in]  pitChannelNum   Channel to configure
    *  @param[in]  callback  Callback function to be executed on interrupt.\n
    *                        Use nullptr to remove callback.
    */
   static void setCallback(PitChannelNum pitChannelNum, PitCallbackFunction callback) {
      static_assert(Info::irqHandlerInstalled, "PIT not configure for interrupts - Modify Configure.usbdm");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      sCallbacks[pitChannelNum] = callback;
   }

protected:
   /** Pointer to hardware */
   static __attribute__((always_inline)) volatile PIT_Type &pit()      { return Info::pit(); }

public:
   /**
    * Basic enable of PIT
    */
   static void enable() {
      // Enable clock
      Info::enableClock();
      __DMB();
   }

   /**
    *  Enable the PIT with default settings.
    *  All channels are enabled with default settings.
    */
   static void defaultConfigure() {
      enable();

      // Enable timer
      pit().MCR = Info::mcr;
      for (PitChannelNum channel = PitChannelNum_0;
           channel < PitInfo::NumChannels;
           channel = channel+1) {
         configureChannelInTicks(channel, Info::pit_ldval);
         disableNvicInterrupts(Info::irqLevel);
      }
   }

   /**
    *  Enables and configures the PIT.
    *  This also disables all channel interrupts and channel reservations.
    *
    *  @param[in]  pitDebugMode  Determined whether the PIT halts when suspended during debug
    */
   static void configure(PitDebugMode pitDebugMode=PitDebugMode_Stop) {
      enable();
      for (PitChannelNum channel = PitChannelNum_0;
           channel < PitInfo::NumChannels;
           channel = channel+1) {
         disableNvicInterrupts(channel);
      }
      pit().MCR = pitDebugMode|PIT_MCR_MDIS(0); // MDIS cleared => enabled!
      allocatedChannels = -1;
   }

   /**
    *  Enables and configures the PIT if not already done.
    *  This also disables all channel interrupts and channel reservations if newly configured.
    *
    *  @param[in]  pitDebugMode  Determined whether the PIT halts when suspended during debug
    */
   static void configureIfNeeded(PitDebugMode pitDebugMode=PitDebugMode_Stop) {
      enable();
      // Check if disabled and configure if so
      if ((pit().MCR & PIT_MCR_MDIS_MASK) != 0) {
         configure(pitDebugMode);
      }
   }

   /**
    *   Disable the PIT (all channels)
    */
   static void disable() {
      pit().MCR = PIT_MCR_MDIS(1);
      Info::disableClock();
   }

   /**
    * Enable interrupts in NVIC
    *
    * @param[in]  pitChannelNum       Channel being modified
    */
   static void enableNvicInterrupts(PitChannelNum pitChannelNum) {
      static const IRQn_Type irqNums[] = {
            Info::irqNums[0], Info::irqNums[1], Info::irqNums[2], Info::irqNums[3],
      };
      usbdm_assert(pitChannelNum<Info::irqCount,"Illegal PIT channel");
      NVIC_EnableIRQ(irqNums[pitChannelNum]);
   }

   /**
    * Enable and set priority of interrupts in NVIC
    * Any pending NVIC interrupts are first cleared.
    *
    * @param[in]  pitChannelNum       Channel being modified
    * @param[in]  nvicPriority  Interrupt priority
    */
   static void enableNvicInterrupts(PitChannelNum pitChannelNum, uint32_t nvicPriority) {
      static const IRQn_Type irqNums[] = {
            Info::irqNums[0], Info::irqNums[1], Info::irqNums[2], Info::irqNums[3],
      };
      usbdm_assert(pitChannelNum<Info::irqCount,"Illegal PIT channel");
      enableNvicInterrupt(irqNums[pitChannelNum], nvicPriority);
   }

   /**
    * Disable interrupts in NVIC
    *
    * @param[in]  pitChannelNum       Channel being modified
    */
   static void disableNvicInterrupts(PitChannelNum pitChannelNum) {
      static const IRQn_Type irqNums[] = {
            Info::irqNums[0], Info::irqNums[1], Info::irqNums[2], Info::irqNums[3],
      };
      usbdm_assert(pitChannelNum<Info::irqCount,"Illegal PIT channel");
      NVIC_DisableIRQ(irqNums[pitChannelNum]);
   }
   
   /**
    *  Enable the PIT channel
    *
    *  @param[in]  pitChannelNum   Channel to enable
    */
   static void enableChannel(const PitChannelNum pitChannelNum) {
      pit().CHANNEL[pitChannelNum].TCTRL |= PIT_TCTRL_TEN_MASK;
   }

   /**
    *   Disable the PIT channel
    *
    *   @param[in]  pitChannelNum Channel to disable
    */
   static void disableChannel(PitChannelNum pitChannelNum) {

      // Disable timer channel
      pit().CHANNEL[pitChannelNum].TCTRL &= ~PIT_TCTRL_TEN_MASK;
   }

   /**
    *  Configure the PIT channel
    *
    *  @param[in]  pitChannelNum           Channel to configure
    *  @param[in]  tickInterval      Interval in timer ticks (usually bus clock period)
    *  @param[in]  pitChannelIrq     Whether to enable interrupts
    *
    *  @note The timer channel is disabled before configuring so that period changes have
    *        immediate effect.
    */
   static void configureChannelInTicks(
         PitChannelNum     pitChannelNum,
         uint32_t          tickInterval,
         PitChannelIrq     pitChannelIrq=PitChannelIrq_Disabled) {

      usbdm_assert(tickInterval>0, "Interval too short");

      pit().CHANNEL[pitChannelNum].TCTRL = 0;
      pit().CHANNEL[pitChannelNum].LDVAL = tickInterval-1;
      pit().CHANNEL[pitChannelNum].TFLG  = PIT_TFLG_TIF_MASK;
      pit().CHANNEL[pitChannelNum].TCTRL = pitChannelIrq|PIT_TCTRL_TEN(1);
   }

   /**
    *  Configure the PIT channel in seconds
    *
    *  @param[in]  pitChannelNum     Channel to configure
    *  @param[in]  intervalInSeconds Interval in seconds
    *  @param[in]  pitChannelIrq      Whether to enable interrupts
    *
    *  @note The timer channel is disabled before configuring so that period changes have
    *        immediate effect.
    */
   static void configureChannel(
         PitChannelNum     pitChannelNum,
         float             intervalInSeconds,
         PitChannelIrq     pitChannelIrq=PitChannelIrq_Disabled) {

      configureChannelInTicks(pitChannelNum, convertSecondsToTicks(intervalInSeconds), pitChannelIrq);
   }

   /**
    *  Configure the PIT channel in milliseconds
    *
    *  @param[in]  pitChannelNum    Channel to configure
    *  @param[in]  milliseconds     Interval in seconds
    *  @param[in]  pitChannelIrq    Whether to enable interrupts
    *
    *  @note The timer channel is disabled before configuring so that period changes have
    *        immediate effect.
    */
   static void configureChannelInMilliseconds(
         PitChannelNum     pitChannelNum,
         unsigned          milliseconds,
         PitChannelIrq     pitChannelIrq=PitChannelIrq_Disabled) {

      configureChannelInTicks(pitChannelNum, convertMillisecondsToTicks(milliseconds), pitChannelIrq);
   }

   /**
    *  Configure the PIT channel in microseconds
    *
    *  @param[in]  pitChannelNum    Channel to configure
    *  @param[in]  microseconds     Interval in microseconds
    *  @param[in]  pitChannelIrq    Whether to enable interrupts
    *
    *  @note The timer channel is disabled before configuring so that period changes have
    *        immediate effect.
    */
   static void configureChannelInMicroseconds(
         PitChannelNum     pitChannelNum,
         unsigned          microseconds,
         PitChannelIrq     pitChannelIrq=PitChannelIrq_Disabled) {

      configureChannelInTicks(pitChannelNum, convertMicrosecondsToTicks(microseconds), pitChannelIrq);
   }

   /**
    * Convert time in ticks to time in seconds
    *
    * @param[in] ticks Time interval in ticks
    *
    * @return Time interval in seconds
    */
   static float convertTicksToSeconds(uint32_t ticks) {
      return ((float)ticks)/Info::getClockFrequency();
   }

   /**
    * Convert time in ticks to time in microseconds
    *
    * @param[in] ticks Time interval in ticks
    *
    * @return Time interval in microseconds
    */
   static unsigned convertTicksToMilliseconds(uint32_t ticks) {
      return (unsigned)((1000UL * ticks)/Info::getClockFrequency());
   }

   /**
    * Convert time in ticks to time in milliseconds
    *
    * @param[in] ticks Time interval in ticks
    *
    * @return Time interval in milliseconds
    */
   static unsigned convertTicksToMicroseconds(uint32_t ticks) {
      return (unsigned)((1000000UL * ticks)/Info::getClockFrequency());
   }

   /**
    * Converts time in seconds to time in ticks
    *
    * @param[in] seconds Time interval in seconds
    *
    * @return Time interval in ticks
    *
    * @note Will set error code if calculated value is unsuitable
    */
   static int convertSecondsToTicks(float seconds) {
      float intervalInTicks = rintf(seconds*Info::getClockFrequency());
      usbdm_assert(intervalInTicks <= 0xFFFFFFFFUL, "Interval is too long");
      usbdm_assert(intervalInTicks > 0, "Interval is too short");
      if (intervalInTicks > 0xFFFFFFFFUL) {
         setErrorCode(E_TOO_LARGE);
      }
      if (intervalInTicks <= 0) {
         setErrorCode(E_TOO_SMALL);
      }
      return rintf((uint32_t)intervalInTicks);
   }

   /**
    * Converts time in seconds to time in ticks
    *
    * @param[in] milliseconds Time interval in milliseconds
    *
    * @return Time interval in ticks
    *
    * @note Will set error code if calculated value is unsuitable
    */
   static int convertMillisecondsToTicks(unsigned milliseconds) {
      unsigned long intervalInTicks = milliseconds*(Info::getClockFrequency()/1000);
      usbdm_assert(intervalInTicks <= 0xFFFFFFFFUL, "Interval is too long");
      usbdm_assert(intervalInTicks > 0, "Interval is too short");
      if (intervalInTicks > 0xFFFFFFFFUL) {
         setErrorCode(E_TOO_LARGE);
      }
      if (intervalInTicks <= 0) {
         setErrorCode(E_TOO_SMALL);
      }
      return (uint32_t)intervalInTicks;
   }

   /**
    * Converts time in seconds to time in ticks
    *
    * @param[in] microseconds Time interval in microseconds
    *
    * @return Time interval in ticks
    *
    * @note Will set error code if calculated value is unsuitable
    */
   static int convertMicrosecondsToTicks(unsigned microseconds) {
      unsigned long intervalInTicks = microseconds*(Info::getClockFrequency()/1000000);
      usbdm_assert(intervalInTicks <= 0xFFFFFFFFUL, "Interval is too long");
      usbdm_assert(intervalInTicks > 0, "Interval is too short");
      if (intervalInTicks > 0xFFFFFFFFUL) {
         setErrorCode(E_TOO_LARGE);
      }
      if (intervalInTicks <= 0) {
         setErrorCode(E_TOO_SMALL);
      }
      return (uint32_t)intervalInTicks;
   }

   /**
    * Set period in seconds
    *
    * @param[in]  pitChannelNum Channel being modified
    * @param[in]  ticks         Interval in ticks
    *
    * @note If the timer is currently enabled this value will be loaded on the next expiration.
    *       To have immediate effect it is necessary to use configureChannel().
    */
   static void setPeriodInTicks(PitChannelNum pitChannelNum, uint32_t ticks) {
      pit().CHANNEL[pitChannelNum].LDVAL = ticks-1;
   }

   /**
    * Set period in microseconds
    *
    * @param[in]  pitChannelNum Channel being modified
    * @param[in]  microseconds  Interval in microseconds
    *
    * @note If the timer is currently enabled this value will be loaded on the next expiration.
    *       To have immediate effect it is necessary to use configureChannel().
    */
   static void setPeriodInMicroseconds(PitChannelNum pitChannelNum, uint32_t microseconds) {
      setPeriodInTicks(pitChannelNum, convertMicrosecondsToTicks(microseconds));
   }

   /**
    * Set period in milliseconds
    *
    * @param[in]  pitChannelNum Channel being modified
    * @param[in]  milliseconds  Interval in milliseconds
    *
    * @note If the timer is currently enabled this value will be loaded on the next expiration.
    *       To have immediate effect it is necessary to use configureChannel().
    */
   static void setPeriodInMilliseconds(PitChannelNum pitChannelNum, uint32_t milliseconds) {
      setPeriodInTicks(pitChannelNum, convertMillisecondsToTicks(milliseconds));
   }

   /**
    * Set period in seconds
    *
    * @param[in]  pitChannelNum Channel being modified
    * @param[in]  interval Interval in seconds
    *
    * @note If the timer is currently enabled this value will be loaded on the next expiration.
    *       To have immediate effect it is necessary to use configureChannel().
    */
   static void setPeriod(PitChannelNum pitChannelNum, float interval) {
      setPeriodInTicks(pitChannelNum, rintf(interval*Info::getClockFrequency()));
   }

   /**
    *  Use a PIT channel to implement a busy-wait delay
    *
    *  @param[in]  pitChannelNum   Channel to use
    *  @param[in]  interval  Interval to wait in timer ticks (usually bus clock period)
    *
    *  @note Function doesn't return until interval has expired
    */
   static void delayInTicks(PitChannelNum pitChannelNum, uint32_t interval) {
      configureChannelInTicks(pitChannelNum, interval);
      while (pit().CHANNEL[pitChannelNum].TFLG == 0) {
         __NOP();
      }
      disableChannel(pitChannelNum);
   }

   /**
    *  Use a PIT channel to implement a busy-wait delay
    *
    *  @param[in]  pitChannelNum   Channel to use
    *  @param[in]  interval  Interval to wait as a float
    *
    *  @note Function doesn't return until interval has expired
    */
   static void delay(PitChannelNum pitChannelNum, float interval) {
      configureChannel(pitChannelNum, interval);
      while (pit().CHANNEL[pitChannelNum].TFLG == 0) {
         __NOP();
      }
      disableChannel(pitChannelNum);
   }

   /**
    * Set one-shot timer callback.
    *
    *  @note It is necessary to enable NVIC interrupts beforehand
    *
    *  @param[in]  pitChannelNum     Channel to configure
    *  @param[in]  callbackFunction  Function to call from stub ISR
    *  @param[in]  interval          Interval in seconds until callback is executed
    */
   static void oneShot(PitChannelNum pitChannelNum, PitCallbackFunction callbackFunction, float interval) {
      clearOnEvent |= (1<<pitChannelNum);
      setCallback(pitChannelNum, callbackFunction);
      configureChannel(pitChannelNum, interval, PitChannelIrq_Enabled);
   }

   /**
    * Set one-shot timer callback in microseconds
    *
    *  @note It is necessary to enable NVIC interrupts beforehand
    *
    *  @param[in]  pitChannelNum     Channel to configure
    *  @param[in]  callbackFunction  Function to call from stub ISR
    *  @param[in]  microseconds      Interval in milliseconds
    */
   static void oneShotInMicroseconds(PitChannelNum pitChannelNum, PitCallbackFunction callbackFunction, uint32_t microseconds) {
      clearOnEvent |= (1<<pitChannelNum);
      setCallback(pitChannelNum, callbackFunction);
      configureChannelInMicroseconds(pitChannelNum, microseconds, PitChannelIrq_Enabled);
   }

   /**
    * Set one-shot timer callback in milliseconds
    *
    *  @note It is necessary to enable NVIC interrupts beforehand
    *
    *  @param[in]  pitChannelNum     Channel to configure
    *  @param[in]  callbackFunction  Function to call from stub ISR
    *  @param[in]  milliseconds      Interval in milliseconds
    */
   static void oneShotInMilliseconds(PitChannelNum pitChannelNum, PitCallbackFunction callbackFunction, uint32_t milliseconds) {
      clearOnEvent |= (1<<pitChannelNum);
      setCallback(pitChannelNum, callbackFunction);
      configureChannelInMilliseconds(pitChannelNum, milliseconds, PitChannelIrq_Enabled);
   }

   /**
    * Set one-shot timer callback
    *
    *  @note It is necessary to enable NVIC interrupts beforehand
    *
    *  @param[in]  pitChannelNum           Channel to configure
    *  @param[in]  callbackFunction  Function to call from stub ISR
    *  @param[in]  tickInterval      Interval in timer ticks (usually bus clock period)
    */
   static void oneShotInTicks(PitChannelNum pitChannelNum, PitCallbackFunction callbackFunction, uint32_t tickInterval) {
      clearOnEvent |= (1<<pitChannelNum);
      setCallback(pitChannelNum, callbackFunction);
      configureChannelInTicks(pitChannelNum, tickInterval, PitChannelIrq_Enabled);
   }

   /**
    * Class representing a PIT channel
    *
    * @tparam channel Timer channel number
    */
   template <int channel>
   class Channel {

   public:
      /** Timer channel number */
      static constexpr PitChannelNum CHANNEL = (PitChannelNum)channel;

      /**
       * Set interrupt callback
       *
       * @param[in]  callbackFunction  Function to call from stub ISR
       */
      static void setCallback(PitCallbackFunction callbackFunction) {
         PitBase_T<Info>::setCallback(CHANNEL, callbackFunction);
      }

      /** PIT interrupt handler - Calls PIT callback */
      static void irqHandler() {
         // Clear interrupt flag
         PitBase_T<Info>::pit().CHANNEL[channel].TFLG = PIT_TFLG_TIF_MASK;

         if (clearOnEvent&(1<<channel)) {
            disable();
            clearOnEvent &= ~(1<<channel);
         }
         sCallbacks[channel]();
      }

      /**
       *  Configure the PIT channel
       *
       *  @param[in]  interval          Interval in timer ticks (usually bus clock)
       *  @param[in]  pitChannelIrq     Whether to enable interrupts
       *
       *  @note The timer channel is disabled before configuring so that period changes
       *        have immediate effect.
       */
      static void __attribute__((always_inline)) configureInTicks(
            uint32_t          interval,
            PitChannelIrq     pitChannelIrq=PitChannelIrq_Disabled) {

         PitBase_T<Info>::configureChannelInTicks(CHANNEL, interval, pitChannelIrq);
      }

      /**
       *  Configure the PIT channel
       *
       *  @param[in]  interval          Interval in seconds
       *  @param[in]  pitChannelIrq     Whether to enable interrupts
       *
       *  @note The timer channel is disabled before configuring so that period changes
       *        have immediate effect.
       */
      static void __attribute__((always_inline)) configure(
            float             interval,
            PitChannelIrq     pitChannelIrq=PitChannelIrq_Disabled) {

         PitBase_T<Info>::configureChannel(CHANNEL, interval, pitChannelIrq);
      }

      /**
       * Set period in seconds
       *
       * @param[in]  interval Interval in seconds
       *
       * @note If the timer is currently enabled this value will be loaded on the next expiration.
       *       To have immediate effect it is necessary to use configure().
       */
      static void __attribute__((always_inline)) setPeriod(float interval) {
         PitBase_T<Info>::setPeriod(CHANNEL, interval);
      }

      /**
       * Set period in ticks
       *
       * @param[in]  interval Interval in ticks
       *
       * @note If the timer is currently enabled this value will be loaded on the next expiration.
       *       To have immediate effect it is necessary to use configure().
       */
      static void __attribute__((always_inline)) setPeriodInTicks(uint32_t interval) {
         PitBase_T<Info>::setPeriodInTicks(CHANNEL, interval);
      }

      /**
       * Set period in microseconds
       *
       * @param[in]  microseconds Interval in microseconds
       *
       * @note If the timer is currently enabled this value will be loaded on the next expiration.
       *       To have immediate effect it is necessary to use configure().
       */
      static void __attribute__((always_inline)) setPeriodInMicroseconds(uint32_t microseconds) {
         unsigned long interval = ((unsigned long)microseconds*PitInfo::getClockFrequency())/1000000;
         usbdm_assert(interval<0xFFFFFFFFUL,"Interval too long");
         PitBase_T<Info>::setPeriodInTicks(CHANNEL, microseconds);
      }

      /**
       *  Enables and configures the PIT if not already done.
       *  This also disables all channel interrupts and channel reservations if newly configured.
       *
       *  @param[in]  pitDebugMode  Determined whether the PIT halts when suspended during debug
       */
      static void  __attribute__((always_inline)) configureIfNeeded(PitDebugMode pitDebugMode=PitDebugMode_Stop) {
         PitBase_T<Info>::configureIfNeeded(pitDebugMode);
      }

      /**
       *   Enable the PIT channel
       */
      static void __attribute__((always_inline)) enable() {
         PitBase_T<Info>::enableChannel(CHANNEL);
      }

      /**
       *   Disable the PIT channel
       */
      static void __attribute__((always_inline)) disable() {
         PitBase_T<Info>::disableChannel(CHANNEL);
      }

      /**
       * Enable/disable channel interrupts.
       *
       * @param[in]  enable  True => enable, False => disable
       *
       * @note It is also necessary to modify NVIC using enableNvicInterrupts().
       */
      static void __attribute__((always_inline)) enableInterrupts(bool enable=true) {
         PitBase_T<Info>::enableInterrupts(CHANNEL, enable);
      }

      /**
       * Enable interrupts in NVIC
       */
      static void enableNvicInterrupts() {
         return PitBase_T<Info>::enableNvicInterrupts(CHANNEL);
      }

      /**
       * Enable and set priority of interrupts in NVIC
       * Any pending NVIC interrupts are first cleared.
       *
       * @param[in]  nvicPriority  Interrupt priority
       */
      static void enableNvicInterrupts(uint32_t nvicPriority) {
         return PitBase_T<Info>::enableNvicInterrupts(CHANNEL, nvicPriority);
      }

      /**
       * Disable interrupts in NVIC
       */
      static void disableNvicInterrupts() {
         return PitBase_T<Info>::disableNvicInterrupts(CHANNEL);
      }

      /**
       *  Use a PIT channel to implement a busy-wait delay
       *
       *  @param[in]  interval  Interval to wait in timer ticks (usually bus clock period)
       *
       *  @note Function doesn't return until interval has expired
       */
      static void __attribute__((always_inline)) delayInTicks(uint32_t interval) {
         PitBase_T<Info>::delayInTicks(CHANNEL, interval);
      }

      /**
       *  Use a PIT channel to implement a busy-wait delay
       *
       *  @param[in]  interval  Interval to wait as a float
       *
       *  @note Function doesn't return until interval has expired
       */
      static void __attribute__((always_inline)) delay(float interval) {
         PitBase_T<Info>::delay(CHANNEL, interval);
      }

      /**
       * Set one-shot timer callback.
       *
       *  @note It is necessary to enable NVIC interrupts beforehand
       *
       *  @param[in]  callbackFunction  Function to call from stub ISR
       *  @param[in]  interval          Interval in seconds until callback is executed
       */
      static void  __attribute__((always_inline)) oneShot(PitCallbackFunction callbackFunction, float interval) {
         PitBase_T<Info>::oneShot(CHANNEL, callbackFunction, interval);
      }

      /**
       * Set one-shot timer callback in microseconds
       *
       *  @note It is necessary to enable NVIC interrupts beforehand
       *
       *  @param[in]  pitChannelNum     Channel to configure
       *  @param[in]  callbackFunction  Function to call from stub ISR
       *  @param[in]  microseconds      Interval in milliseconds
       */
      static void __attribute__((always_inline)) oneShotInMicroseconds(PitCallbackFunction callbackFunction, uint32_t microseconds) {
         PitBase_T<Info>::oneShotInMicroseconds(CHANNEL, callbackFunction, microseconds);
      }

      /**
       * Set one-shot timer callback in milliseconds
       *
       *  @note It is necessary to enable NVIC interrupts beforehand
       *
       *  @param[in]  pitChannelNum     Channel to configure
       *  @param[in]  callbackFunction  Function to call from stub ISR
       *  @param[in]  milliseconds      Interval in milliseconds
       */
      static void __attribute__((always_inline)) oneShotInMilliseconds(PitCallbackFunction callbackFunction, uint32_t milliseconds) {
         PitBase_T<Info>::oneShotInMilliseconds(CHANNEL, callbackFunction, milliseconds);
      }

      /**
       * Set one-shot timer callback
       *
       *  @note It is necessary to enable NVIC interrupts beforehand
       *
       *  @param[in]  pitChannelNum           Channel to configure
       *  @param[in]  callbackFunction  Function to call from stub ISR
       *  @param[in]  tickInterval      Interval in timer ticks (usually bus clock period)
       */
      static void __attribute__((always_inline)) oneShotInTicks(PitCallbackFunction callbackFunction, uint32_t tickInterval) {
         PitBase_T<Info>::oneShotInTicks(CHANNEL, callbackFunction, tickInterval);
      }
   };
};

//template<class Info>
//   bool PitBase_T<Info>::  template<> Channel<channel>::clearOnEvent = false;


/** Bit-mask of allocated channels */
template<class Info> uint32_t PitBase_T<Info>::allocatedChannels = -1;

/**
 * Callback table for programmatically set handlers
 */
template<class Info>
PitCallbackFunction PitBase_T<Info>::sCallbacks[] = {
      PitBase_T<Info>::unhandledCallback,
      PitBase_T<Info>::unhandledCallback,
      PitBase_T<Info>::unhandledCallback,
      PitBase_T<Info>::unhandledCallback,
};

template<class Info>
uint8_t PitBase_T<Info>::clearOnEvent = 0;

#ifdef PIT
/**
 * @brief class representing the PIT
 */
using Pit = PitBase_T<PitInfo>;
#endif

/**
 * @}
 */

} // End namespace USBDM

#endif /* INCLUDE_USBDM_PIT_H_ */
