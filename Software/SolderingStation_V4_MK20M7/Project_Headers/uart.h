/**
 * @file     uart.h (180.ARM_Peripherals/Project_Headers/uart.h)
 * @brief    Universal Asynchronous Receiver/Transmitter interface
 *
 * @version  V4.12.1.210
 * @date     13 April 2016
 *      Author: podonoghue
 */

#ifndef INCLUDE_USBDM_UART_H_
#define INCLUDE_USBDM_UART_H_
/*
 * *****************************
 * *** DO NOT EDIT THIS FILE ***
 * *****************************
 *
 * This file is generated automatically.
 * Any manual changes will be lost.
 */
#include <stdint.h>
#include "derivative.h"
#include "pin_mapping.h"
#include "formatted_io.h"
#include "uart_queue.h"
#ifdef __CMSIS_RTOS
#include "cmsis.h"
#endif

namespace USBDM {

/**
 * @addtogroup UART_Group UART, Universal Asynchronous Receiver/Transmitter
 * @brief C++ Class allowing access to UART interface
 * @{
 */

/**
 * Enumeration selecting interrupt sources
 */
enum UartInterrupt {
   UartInterrupt_TxHoldingEmpty  = UART_C2_TIE(1),   //!< Interrupt request on Transmit holding register empty
   UartInterrupt_TxComplete      = UART_C2_TCIE(1),  //!< Interrupt request on Transmit complete
   UartInterrupt_RxFull          = UART_C2_RIE(1),   //!< Interrupt request on Receive holding full
   UartInterrupt_IdleDetect      = UART_C2_ILIE(1),  //!< Interrupt request on Idle detection
};

/**
 * Enumeration selecting direct memory access sources
 */
enum UartDma {
#ifdef UART_C5_TDMAS
   UartDma_TxHoldingEmpty  = UART_C5_TDMAS(1),   //!< DMA request on Transmit holding register empty
   UartDma_RxFull          = UART_C5_RDMAS(1),   //!< DMA request on Receive holding full
#endif
#ifdef UART_C5_TDMAE
   UartDma_TxHoldingEmpty  = UART_C5_TDMAE(1),   //!< DMA request on Transmit holding register empty
   UartDma_RxFull          = UART_C5_RDMAE(1),   //!< DMA request on Receive holding full
#endif
};

/**
 * @brief Virtual Base class for UART interface
 */
class Uart : public FormattedIO {

private:
   Uart() = delete;
   Uart(const Uart&) = delete;
   Uart(Uart&&) = delete;

protected:
#ifdef __CMSIS_RTOS
   /**
    * Obtain UART mutex.
    *
    * @param[in]  milliseconds How long to wait in milliseconds. Use osWaitForever for indefinite wait
    *
    * @return osOK:                    The mutex has been obtain.
    * @return osErrorTimeoutResource:  The mutex could not be obtained in the given time.
    * @return osErrorResource:         The mutex could not be obtained when no timeout was specified.
    * @return osErrorParameter:        The parameter mutex_id is incorrect.
    * @return osErrorISR:              Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus startTransaction(int milliseconds=osWaitForever) = 0;

   /**
    * Release UART mutex.
    *
    * @return osOK:              The mutex has been correctly released.
    * @return osErrorResource:   The mutex was not obtained before.
    * @return osErrorISR:        Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus endTransaction() = 0;
#else
   /**
    * Obtain UART - dummy routine (non RTOS)
    */
   int startTransaction(int =0) {
      return 0;
   }
   /**
    * Release UART - dummy routine (non RTOS)
    */
   int endTransaction() {
      return 0;
   }
#endif

   /**
    * Check if character is available
    *
    * @return true  Character available i.e. _readChar() will not block
    * @return false No character available
    */
   virtual bool _isCharAvailable() override {
      return (uart->S1 & UART_S1_RDRF_MASK);
   }

   /**
    * Receives a single character (blocking)
    *
    * @return Character received
    */
   virtual int _readChar() override {

      // Get status from UART
      uint8_t status;
      do {
         // Get status from UART
         status = uart->S1;
         // Clear & ignore pending errors
         if ((status & (UART_S1_FE_MASK|UART_S1_OR_MASK|UART_S1_PF_MASK|UART_S1_NF_MASK)) != 0) {
            clearError();
         }
         // Check for Rx buffer full
      } while ((status & UART_S1_RDRF_MASK) == 0);
      return (uint8_t)(uart->D);
   }

   /**
    * Writes a character (blocking)
    *
    * @param[in]  ch - character to send
    */
   virtual void _writeChar(char ch) override {
      while ((uart->S1 & UART_S1_TDRE_MASK) == 0) {
         // Wait for Tx buffer empty
         __asm__("nop");
      }
      uart->D = ch;
      if (ch=='\n') {
         write('\r');
      }
   }

   /**
    * Handler for interrupts when no handler set
    */
   static void unhandledCallback(uint8_t) {
      setAndCheckErrorCode(E_NO_HANDLER);
   }

public:

   /**
    * UART hardware instance
    */
   const HardwarePtr<UART_Type> uart;
   
   /**
    * Construct UART interface
    *
    * @param[in]  baseAddress Base address of UART hardware
    */
   Uart(uint32_t baseAddress) : uart((baseAddress)) {
   }

   /**
    * Destructor
    */
   virtual ~Uart() {
   }

#ifdef UART_C4_BRFA_MASK
   /**
    * Set baud factor value for interface.
    * For UARTS with baud rate fraction adjust (BRFA) support.
    *
    * This is calculated from baud rate and UART clock frequency
    *
    * @param[in]  baudrate       Interface speed in bits-per-second
    * @param[in]  clockFrequency Frequency of UART clock
    */
   void __attribute__((noinline)) setBaudRate_brfa(uint32_t baudrate, uint32_t clockFrequency) {

      /*
       * Baudrate = clockFrequency / (OSR x (SBR + BRFD))
       * Fixed OSR = 16
       *
       * (OSR x (SBR + BRFD/32)) = clockFrequency/Baudrate
       * (SBR + BRFD/32) = clockFrequency/(Baudrate*OSR)
       * 32*SBR + BRFD = 2*clockFrequency/Baudrate
       * SBR  = (2*clockFrequency/Baudrate)>>5
       * BRFD = (2*clockFrequency/Baudrate)&0x1F
       */
      // Disable UART before changing registers
      uint8_t c2Value = uart->C2;
      uart->C2 = 0;

      // Calculate UART clock setting (5-bit fraction at right)
      int divider = (2*clockFrequency)/baudrate;

      // Set Baud rate register
      uart->BDH = (uart->BDH&~UART_BDH_SBR_MASK) | UART_BDH_SBR((divider>>(8+5)));
      uart->BDL = UART_BDL_SBR(divider>>5);
      // Fractional divider to get closer to the baud rate
      uart->C4 = (uart->C4&~UART_C4_BRFA_MASK) | UART_C4_BRFA(divider);

      // Restore UART settings
      uart->C2 = c2Value;
   }
#endif

   /**
    * Set baud factor value for interface.
    * For basic UARTS.
    *
    * This is calculated from baud rate and UART clock frequency
    *
    * @param[in]  baudrate       Interface speed in bits-per-second
    * @param[in]  clockFrequency Frequency of UART clock
    * @param[in]  oversample     Over-sample ratio to use when calculating divider
    */
   void __attribute__((noinline)) setBaudRate_basic(uint32_t baudrate, uint32_t clockFrequency, uint32_t oversample) {

      /*
       * Baudrate = ClockFrequency / (OverSample x Divider)
       * Divider = ClockFrequency / (OverSample x Baudrate)
       */

      // Disable UART before changing registers
      uint8_t c2Value = uart->C2;
      uart->C2 = 0;

      // Calculate UART divider with rounding
      uint32_t divider = (clockFrequency<<1)/(oversample * baudrate);
      divider = (divider>>1)|(divider&0b1);

      // Set Baud rate register
      uart->BDH = (uart->BDH&~UART_BDH_SBR_MASK) | UART_BDH_SBR((divider>>8));
      uart->BDL = UART_BDL_SBR(divider);

      // Restore UART settings
      uart->C2 = c2Value;
   }

   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and LPUART clock frequency
    *
    * @param[in]  baudrate  Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) = 0;

   /**
    * Clear UART error status
    */
   virtual void clearError() = 0;

   /**
    * Enable/disable an interrupt source
    *
    * @param[in] uartInterrupt Interrupt source to modify
    * @param[in] enable        True to enable, false to disable
    *
    * @note Changing the enabled interrupt functions may also affect the DMA settings
    */
   void enableInterrupt(UartInterrupt uartInterrupt, bool enable=true) {
      if (enable) {
#ifdef UART_C5_TDMAS
         uart->C5 &= ~uartInterrupt; // DMA must be off to enable interrupts
#endif
         uart->C2 |= uartInterrupt;
      }
      else {
         uart->C2 &= ~uartInterrupt; // May also disable DMA
      }
   }

   /**
    * Enable/disable a DMA source
    *
    * @param[in] uartDma  DMA source to modify
    * @param[in] enable   True to enable, false to disable
    *
    * @note Changing the enabled DMA functions may also affect the interrupt settings
    */
   void enableDma(UartDma uartDma, bool enable=true) {
      // Flags are in same positions in the C2 and C5
      if (enable) {
         uart->C5 |= uartDma;
#ifdef UART_C5_TDMAS
         uart->C2 |= uartDma; // Interrupts must be enable for DMA
#endif
      }
      else {
#ifdef UART_C5_TDMAS
         uart->C2 &= ~uartDma; // Switching DMA off shouldn't enable interrupts!
#endif
         uart->C5 &= ~uartDma;
      }
   }

   /**
    *  Flush output data
    */
   virtual Uart &flushOutput() override {
      while ((uart->S1 & UART_S1_TC_MASK) == 0) {
      // Wait until transmission of last character is complete
      }
      return *this;
   };

   /**
    *  Flush input data
    */
   virtual Uart &flushInput() override {
      (void)uart->D;
      return (Uart &)FormattedIO::flushInput();
   };
};

/**
 * Type definition for UART interrupt call back
 *
 *  @param[in]  status - Interrupt flags e.g. UART_S1_TDRE, UART_S1_RDRF etc
 */
typedef void (*UARTCallbackFunction)(uint8_t status);

/**
 * @brief Template class representing an UART interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  Uart *uart0 = new USBDM::Uart_T<Uart0Info>(115200);
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 *
 * @tparam Info   Class describing UART hardware
 */
template<class Info> class Uart_T : public Uart {

private:
   Uart_T(const Uart_T&) = delete;
   Uart_T(Uart_T&&) = delete;

public:
   /** Get reference to UART hardware as struct */
   static volatile UART_Type &uartPtr() { return Info::uart(); }

   /** Get base address of LPUART hardware as uint32_t */
   static constexpr uint32_t uartBase() { return Info::baseAddress; }

   /** Get base address of UART.D register as uint32_t */
   static constexpr uint32_t uartD() { return uartBase() + offsetof(UART_Type, D); }

#ifdef __CMSIS_RTOS
protected:
   /**
    * Mutex to protect access\n
    * Using a static accessor function avoids issues with static object initialisation order
    *
    * @return mutex
    */
   static CMSIS::Mutex &mutex(int =0) {
      /** Mutex to protect access - static so per UART */
      static CMSIS::Mutex mutex;
      return mutex;
   }

public:
   /**
    * Obtain UART mutex.
    *
    * @param[in]  milliseconds How long to wait in milliseconds. Use osWaitForever for indefinite wait
    *
    * @return osOK:                    The mutex has been obtain.
    * @return osErrorTimeoutResource:  The mutex could not be obtained in the given time.
    * @return osErrorResource:         The mutex could not be obtained when no timeout was specified.
    * @return osErrorParameter:        The parameter mutex_id is incorrect.
    * @return osErrorISR:              Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus startTransaction(int milliseconds=osWaitForever) override {
      // Obtain mutex
      osStatus status = mutex().wait(milliseconds);
      if (status != osOK) {
         CMSIS::setCmsisErrorCode(status);
      }
      return status;
   }

   /**
    * Release UART mutex
    *
    * @return osOK:              The mutex has been correctly released.
    * @return osErrorResource:   The mutex was not obtained before.
    * @return osErrorISR:        Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus endTransaction() override {
      // Release mutex
      osStatus status = mutex().release();
      if (status != osOK) {
         CMSIS::setCmsisErrorCode(status);
      }
      return status;
   }
#endif

protected:
   /** Callback function for RxTx ISR */
   static UARTCallbackFunction rxTxCallback;
   /** Callback function for Error ISR */
   static UARTCallbackFunction errorCallback;
   /** Callback function for LON ISR */
   static UARTCallbackFunction lonCallback;

public:
   // Template _mapPinsOption_on.xml

   /**
    * Configures all mapped pins associated with UART
    */
   static void configureAllPins() {
   
      // Configure pins if selected and not already locked
      if constexpr (Info::mapPinsOnEnable && !(MapAllPinsOnStartup && (ForceLockedPins == PinLock_Locked))) {
         Info::initPCRs();
      }
   }

   /**
    * Disabled all mapped pins associated with UART
    *
    * @note Only the lower 16-bits of the PCR registers are modified
    */
   static void disableAllPins() {
   
      // Disable pins if selected and not already locked
      if constexpr (Info::mapPinsOnEnable && !(MapAllPinsOnStartup && (ForceLockedPins == PinLock_Locked))) {
      Info::clearPCRs();
      }
   }

   /**
    * Basic enable of UART
    * Includes enabling clock and configuring all pins if mapPinsOnEnable is selected in configuration
    */
   static void enable() {
   
      // Enable clock to peripheral
      Info::enableClock();
   
      configureAllPins();
   }

   /**
    * Disables the clock to UART and all mappable pins
    */
   static void disable() {
   
      disableNvicInterrupts();
      
      disableAllPins();
   
      // Disable clock to peripheral
      Info::disableClock();
   }
// End Template _mapPinsOption_on.xml


   /**
    * Construct UART interface
    */
   Uart_T() : Uart(Info::baseAddress) {
      // Check pin assignments
      static_assert(Info::info[0].gpioBit >= 0, "Uart_Tx has not been assigned to a pin - Modify Configure.usbdm");
      static_assert(Info::info[1].gpioBit >= 0, "Uart_Rx has not been assigned to a pin - Modify Configure.usbdm");
      
      initialise();
   }

   void initialise() {

      // Enable clock to UART interface
      Info::enableClock();

      if constexpr (Info::mapPinsOnEnable) {
         configureAllPins();
      }
      uart->C2 = UART_C2_TE(1)|UART_C2_RE(1);
   }

   /**
    * Destructor
    */
   ~Uart_T() {}

protected:
   /**
    * Clear UART error status
    */
   virtual void clearError() override {
      if (Info::statusNeedsWrite) {
         uart->S1 = 0xFF;
      }
      else {
         (void)uart->D;
      }
   }

public:
   /**
    * Receive/Transmit IRQ handler (MKL)
    */
   static void irqHandler() {
      uint8_t status = Info::uart->S1;
      rxTxCallback(status);
   }

   /**
    * Receive/Transmit IRQ handler (MK)
    */
   static void irqRxTxHandler() {
      uint8_t status = Info::uart->S1;
      rxTxCallback(status);
   }

   /**
    * Error and LON event IRQ handler (MK)
    */
   static void irqErrorHandler() {
      uint8_t status = Info::uart->S1;
      errorCallback(status);
   }

   /**
    * LON IRQ handler (MK)
    */
   static void irqLonHandler() {
      uint8_t status = Info::uart->S1;
      lonCallback(status);
   }

   /**
    * Set Receive/Transmit Callback function
    *
    *  @param[in]  callback  Callback function to be executed on Rx or Tx interrupt.\n
    *                        Use nullptr to remove callback.
    */
   static void setRxTxCallback(UARTCallbackFunction callback) {
      usbdm_assert(Info::irqHandlerInstalled, "UART not configure for interrupts");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      rxTxCallback = callback;
   }

   /**
    * Set Error Callback function
    *
    *  @param[in]  callback  Callback function to be executed on error interrupt.\n
    *                        Use nullptr to remove callback.
    */
   static void setErrorCallback(UARTCallbackFunction callback) {
      usbdm_assert(Info::irqHandlerInstalled, "UART not configure for interrupts");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      errorCallback = callback;
   }

   /**
    * Set LON Callback function
    *
    *  @param[in]  callback  Callback function to be executed on LON interrupt.\n
    *                        Use nullptr to remove callback.
    */
   static void setLonCallback(UARTCallbackFunction callback) {
      usbdm_assert(Info::irqHandlerInstalled, "UART not configure for interrupts");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      lonCallback = callback;
   }

   /**
    * Enable interrupts in NVIC
    */
   static void enableNvicInterrupts() {
      NVIC_EnableIRQ(Info::irqNums[0]);
      if constexpr (Info::irqCount>1) {
         NVIC_EnableIRQ(Info::irqNums[1]);
      }
      if constexpr (Info::irqCount>2) {
         NVIC_EnableIRQ(Info::irqNums[2]);
      }
   }

   /**
    * Enable and set priority of interrupts in NVIC
    * Any pending NVIC interrupts are first cleared.
    *
    * @param[in]  nvicPriority  Interrupt priority
    */
   static void enableNvicInterrupts(NvicPriority nvicPriority) {
      enableNvicInterrupt(Info::irqNums[0], nvicPriority);
      if constexpr (Info::irqCount>1) {
          enableNvicInterrupt(Info::irqNums[1], nvicPriority);
      }
      if constexpr (Info::irqCount>2) {
          enableNvicInterrupt(Info::irqNums[2], nvicPriority);
      }
   }

   /**
    * Disable interrupts in NVIC
    */
   static void disableNvicInterrupts() {
      NVIC_DisableIRQ(Info::irqNums[0]);
      if constexpr (Info::irqCount>1) {
         NVIC_DisableIRQ(Info::irqNums[1]);
      }
      if constexpr (Info::irqCount>2) {
         NVIC_DisableIRQ(Info::irqNums[2]);
      }
   }
};

template<class Info> UARTCallbackFunction Uart_T<Info>::rxTxCallback  = unhandledCallback;
template<class Info> UARTCallbackFunction Uart_T<Info>::errorCallback = unhandledCallback;
template<class Info> UARTCallbackFunction Uart_T<Info>::lonCallback   = unhandledCallback;

#ifdef UART_C4_BRFA_MASK
template<class Info> class Uart_brfa_T : public Uart_T<Info> {

private:
   Uart_brfa_T(const Uart_brfa_T&) = delete;
   Uart_brfa_T(Uart_brfa_T&&) = delete;

public:
   /**
    * Construct UART interface
    *
    * @param[in]  baudrate         Interface speed in bits-per-second
    */
   Uart_brfa_T(unsigned baudrate=Info::defaultBaudRate) : Uart_T<Info>() {
      setBaudRate(baudrate);
   }
   /**
    * Destructor
    */
   virtual ~Uart_brfa_T() {
   }
   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and UART clock frequency
    *
    * @param[in]  baudrate Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) override {
      Uart::setBaudRate_brfa(baudrate, Info::getInputClockFrequency());
   }
};
#endif

#ifdef UART_C4_OSR_MASK
template<class Info> class Uart_osr_T : public Uart_T<Info> {

private:
   Uart_osr_T(const Uart_osr_T&) = delete;
   Uart_osr_T(Uart_osr_T&&) = delete;

public:
   using Uart_T<Info>::uart;

   /**
    * Construct UART interface
    *
    * @param[in]  baudrate         Interface speed in bits-per-second
    */
   Uart_osr_T(unsigned baudrate=Info::defaultBaudRate) : Uart_T<Info>() {
      setBaudRate(baudrate);
   }
   /**
    * Destructor
    */
   virtual ~Uart_osr_T() {
   }
   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and UART clock frequency
    *
    * @param[in]  baudrate Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) override {
      static constexpr int OVER_SAMPLE = Info::oversampleRatio;

      // Set oversample ratio and baud rate
      uart->C4 = (uart->C4&~UART_C4_OSR_MASK)|(OVER_SAMPLE-1);
      Uart::setBaudRate_basic(baudrate, Info::getInputClockFrequency(), OVER_SAMPLE);
   }

   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and LPUART clock frequency
    *
    * @param[in]  baudrate    Interface speed in bits-per-second
    * @param[in]  oversample  Over-sample ratio to use when calculating divider
    */
   void setBaudRate(unsigned baudrate, unsigned oversample) {

      // Set oversample ratio and baud rate
      uart->C4 = (uart->C4&~UART_C4_OSR_MASK)|UART_C4_OSR(oversample-1);
      Uart::setBaudRate_basic(baudrate, Info::getInputClockFrequency(), oversample);
   }

};
#endif

template<class Info> class Uart_basic_T : public Uart_T<Info> {

private:
   Uart_basic_T(const Uart_basic_T&) = delete;
   Uart_basic_T(Uart_basic_T&&) = delete;

public:
   /**
    * Construct UART interface
    *
    * @param[in]  baudrate         Interface speed in bits-per-second
    */
   Uart_basic_T(unsigned baudrate=Info::defaultBaudRate) : Uart_T<Info>() {
      setBaudRate(baudrate);
   }
   /**
    * Destructor
    */
   virtual ~Uart_basic_T() {
   }
   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and UART clock frequency
    *
    * @param[in]  baudrate Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) override {
      // Over-sample ratio - fixed in hardware
      static constexpr int OVER_SAMPLE = 16;

      Uart::setBaudRate_basic(baudrate, Info::getInputClockFrequency(), OVER_SAMPLE);
   }
};

/**
 * @brief Template class representing an UART interface with buffered reception
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  Uart *uart0 = new USBDM::UartBuffered_T<Uart0Info, 20, 30>(115200);
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 *
 * @tparam Info   Class describing UART hardware
 */
template<class Info, int rxSize=Info::receiveBufferSize, int txSize=Info::transmitBufferSize>
class UartBuffered_T : public Uart_T<Info> {

private:
   UartBuffered_T(const UartBuffered_T&) = delete;
   UartBuffered_T(UartBuffered_T&&) = delete;

public:
   using Uart_T<Info>::uart;

   UartBuffered_T() : Uart_T<Info>() {
      Uart::enableInterrupt(UartInterrupt_RxFull);
      Uart_T<Info>::enableNvicInterrupts(Info::irqLevel);
   }

   virtual ~UartBuffered_T() {
      Uart::enableInterrupt(UartInterrupt_RxFull,         false);
      Uart::enableInterrupt(UartInterrupt_TxHoldingEmpty, false);
   }

protected:
   /**
    * Queue for Buffered reception (if used)
    */
   static UartQueue<char, rxSize> rxQueue;
   /**
    * Queue for Buffered transmission (if used)
    */
   static UartQueue<char, txSize> txQueue;

   /** Lock variable for writes */
   static volatile uint32_t fWriteLock;

   /** Lock variable for reads */
   static volatile uint32_t fReadLock;

   /**
    * Writes a character (blocking on queue full)
    *
    * @param[in]  ch - character to send
    */
   virtual void _writeChar(char ch) override {
      lock(&fWriteLock);
      // Add character to buffer
      while (!txQueue.enQueueDiscardOnFull(ch)) {
      }
      uart->C2 |= UART_C2_TIE_MASK;
      unlock(&fWriteLock);
      if (ch=='\n') {
        _writeChar('\r');
      }
   }

   /**
    * Receives a single character (blocking on queue empty)
    *
    * @return Character received
    */
   virtual int _readChar() override {
      lock(&fReadLock);
      while (rxQueue.isEmpty()) {
         __asm__("nop");
      }
      char t = rxQueue.deQueue();
      unlock(&fReadLock);
      return t;
   }

   /**
    * Check if character is available
    *
    * @return true  Character available i.e. _readChar() will not block
    * @return false No character available
    */
   virtual bool _isCharAvailable() override {
      return (!rxQueue.isEmpty());
   }

public:
   /**
    * Receive/Transmit IRQ handler (MKL)
    */
   static void irqHandler()  {
      uint8_t status = Info::uart->S1;
      if (status & UART_S1_RDRF_MASK) {
         // Receive data register full - save data
         rxQueue.enQueueDiscardOnFull(Info::uart->D);
      }
      if (status & UART_S1_TDRE_MASK) {
         // Transmitter ready
         if (txQueue.isEmpty()) {
            // No data available - disable further transmit interrupts
            Info::uart->C2 &= ~UART_C2_TIE_MASK;
         }
         else {
            // Transmit next byte
            Info::uart->D = txQueue.deQueue();
         }
      }
   }

   /**
    * Receive/Transmit IRQ handler (MK)
    */
   static void irqRxTxHandler()  {
      uint8_t status = Info::uart->S1;
      if (status & UART_S1_RDRF_MASK) {
         // Receive data register full - save data
         rxQueue.enQueueDiscardOnFull(Info::uart->D);
      }
      if (status & UART_S1_TDRE_MASK) {
         // Transmitter ready
         if (txQueue.isEmpty()) {
            // No data available - disable further transmit interrupts
            Info::uart->C2 &= ~UART_C2_TIE_MASK;
         }
         else {
            // Transmit next byte
            Info::uart->D = txQueue.deQueue();
         }
      }
   }

   /**
    * Error IRQ handler (MK)
    */
   static void irqErrorHandler() {
      // Ignore errors
      clearError();
   }

   /**
    *  Flush output data.
    *  This blocks until all pending data has been sent
    */
   virtual UartBuffered_T &flushOutput() override {
      while (!txQueue.isEmpty()) {
         // Wait until queue empty
      }
      while ((uart->S1 & UART_S1_TC_MASK) == 0) {
         // Wait until transmission of last character is complete
      }
      return *this;
   }

   /**
    *  Flush input data
    */
   virtual UartBuffered_T &flushInput() override {
      rxQueue.clear();
      return (UartBuffered_T &)Uart_T<Info>::flushInput();
   }

};

#ifdef UART_C4_BRFA_MASK
template<class Info, int rxSize=Info::receiveBufferSize, int txSize=Info::transmitBufferSize>
class UartBuffered_brfa_T : public UartBuffered_T<Info, rxSize, txSize> {

private:
   UartBuffered_brfa_T(const UartBuffered_brfa_T&) = delete;
   UartBuffered_brfa_T(UartBuffered_brfa_T&&) = delete;

public:
   /**
    * Construct UART interface
    *
    * @param[in]  baudrate         Interface speed in bits-per-second
    */
   UartBuffered_brfa_T(unsigned baudrate=Info::defaultBaudRate) : UartBuffered_T<Info, rxSize, txSize>() {
      setBaudRate(baudrate);
   }
   /**
    * Destructor
    */
   virtual ~UartBuffered_brfa_T() {
   }
   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and LPUART clock frequency
    *
    * @param[in]  baudrate Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) override {
      Uart::setBaudRate_brfa(baudrate, Info::getInputClockFrequency());
   }
};
#endif

#ifdef UART_C4_OSR_MASK
template<class Info, int rxSize=Info::receiveBufferSize, int txSize=Info::transmitBufferSize>
class UartBuffered_osr_T : public UartBuffered_T<Info, rxSize, txSize> {

private:
   UartBuffered_osr_T(const UartBuffered_osr_T&) = delete;
   UartBuffered_osr_T(UartBuffered_osr_T&&) = delete;

   using UartBuffered_T<Info, rxSize, txSize>::uart;

public:
   /**
    * Construct UART interface
    *
    * @param[in]  baudrate         Interface speed in bits-per-second
    */
   UartBuffered_osr_T(unsigned baudrate=Info::defaultBaudRate) : UartBuffered_T<Info, rxSize, txSize>() {
      setBaudRate(baudrate);
   }
   /**
    * Destructor
    */
   virtual ~UartBuffered_osr_T() {
   }
   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and LPUART clock frequency
    *
    * @param[in]  baudrate Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) override {
      static constexpr int OVER_SAMPLE = Info::oversampleRatio;

      // Set oversample ratio
      uart->C4 = (uart->C4&~UART_C4_OSR_MASK)|(OVER_SAMPLE-1);

      Uart::setBaudRate_basic(baudrate, Info::getInputClockFrequency(), OVER_SAMPLE);
   }
};
#endif

template<class Info, int rxSize=Info::receiveBufferSize, int txSize=Info::transmitBufferSize>
class UartBuffered_basic_T : public UartBuffered_T<Info, rxSize, txSize> {

private:
   UartBuffered_basic_T(const UartBuffered_basic_T&) = delete;
   UartBuffered_basic_T(UartBuffered_basic_T&&) = delete;

public:
   /**
    * Construct UART interface
    *
    * @param[in]  baudrate         Interface speed in bits-per-second
    */
   UartBuffered_basic_T(unsigned baudrate=Info::defaultBaudRate) : UartBuffered_T<Info, rxSize, txSize>() {
      setBaudRate(baudrate);
   }
   /**
    * Destructor
    */
   virtual ~UartBuffered_basic_T() {
   }
   /**
    * Set baud factor value for interface
    *
    * This is calculated from baud rate and LPUART clock frequency
    *
    * @param[in]  baudrate Interface speed in bits-per-second
    */
   virtual void setBaudRate(unsigned baudrate) override {
      // Over-sample ratio - fixed in hardware
      static constexpr int OVER_SAMPLE = 16;

      Uart::setBaudRate_basic(baudrate, Info::getInputClockFrequency(), OVER_SAMPLE);
   }
};

template<class Info, int rxSize, int txSize> UartQueue<char, rxSize> UartBuffered_T<Info, rxSize, txSize>::rxQueue;
template<class Info, int rxSize, int txSize> UartQueue<char, txSize> UartBuffered_T<Info, rxSize, txSize>::txQueue;
template<class Info, int rxSize, int txSize> volatile uint32_t   UartBuffered_T<Info, rxSize, txSize>::fReadLock  = 0;
template<class Info, int rxSize, int txSize> volatile uint32_t   UartBuffered_T<Info, rxSize, txSize>::fWriteLock = 0;

#ifdef USBDM_UART0_IS_DEFINED
/**
 * @brief Class representing UART0 interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  USBDM::Uart0 uart;
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 */
typedef  Uart_brfa_T<Uart0Info> Uart0;
#endif

#ifdef USBDM_UART1_IS_DEFINED
/**
 * @brief Class representing UART1 interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  USBDM::Uart1 uart;
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 */
typedef  Uart_brfa_T<Uart1Info> Uart1;
#endif

#ifdef USBDM_UART2_IS_DEFINED
/**
 * @brief Class representing UART2 interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  USBDM::Uart2 uart;
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 */
typedef  Uart_brfa_T<Uart2Info> Uart2;
#endif

#ifdef USBDM_UART3_IS_DEFINED
/**
 * @brief Class representing UART3 interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  USBDM::Uart3 uart;
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 */
typedef  Symbol '/UART3/uartClass' not found<Uart3Info> Uart3;
#endif

#ifdef USBDM_UART4_IS_DEFINED
/**
 * @brief Class representing UART4 interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  USBDM::Uart4 uart;
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 */
typedef  Symbol '/UART4/uartClass' not found<Uart4Info> Uart4;
#endif

#ifdef USBDM_UART5_IS_DEFINED
/**
 * @brief Class representing UART5 interface
 *
 * <b>Example</b>
 * @code
 *  // Instantiate interface
 *  USBDM::Uart5 uart;
 *
 *  for(int i=0; i++;) {
 *     uart.write("Hello world,").writeln(i);
 *  }
 *  @endcode
 */
typedef  Symbol '/UART5/uartClass' not found<Uart5Info> Uart5;
#endif

/**
 * End UART_Group
 * @}
 */

} // End namespace USBDM

#endif /* INCLUDE_USBDM_UART_H_ */