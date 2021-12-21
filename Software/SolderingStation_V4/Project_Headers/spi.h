/**
 * @file     spi.h (180.ARM_Peripherals/Project_Headers/spi-MK.h)
 * @brief    Serial Peripheral Interface
 *
 * @version  V4.12.1.210
 * @date     13 April 2016
 *      Author: podonoghue
 */

#ifndef INCLUDE_USBDM_SPI_H_
#define INCLUDE_USBDM_SPI_H_
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
#include "hardware.h"
#ifdef __CMSIS_RTOS
#include "cmsis.h"
#endif

namespace USBDM {

/**
 * @addtogroup SPI_Group SPI, Serial Peripheral Interface
 * @brief C++ Class allowing access to SPI interface
 * @{
 */

/**
 * Type definition for interrupt call back
 * @param status Interrupt status value from SPI->SR
 */
typedef void (*SpiCallbackFunction)(uint32_t status);

/**
 * SPI mode - Controls clock polarity and the timing relationship between clock and data
 */
enum SpiMode {
   SpiMode_0 = SPI_CTAR_CPOL(0)|SPI_CTAR_CPHA(0), //!< Active-high clock (idles low), Data is captured on leading edge of SCK and changes on the following edge.
   SpiMode_1 = SPI_CTAR_CPOL(0)|SPI_CTAR_CPHA(1), //!< Active-high clock (idles low), Data is changes on leading edge of SCK and captured on the following edge.
   SpiMode_2 = SPI_CTAR_CPOL(1)|SPI_CTAR_CPHA(0), //!< Active-low clock (idles high), Data is captured on leading edge of SCK and changes on the following edge.
   SpiMode_3 = SPI_CTAR_CPOL(1)|SPI_CTAR_CPHA(1), //!< Active-low clock (idles high), Data is changes on leading edge of SCK and captured on the following edge.
};

/**
 *  Clock polarity
 *  Selects the inactive state of the Serial Communications Clock (SCK).
 */
enum SpiPolarity {
   SpiPolarity_InactiveLow   = SPI_CTAR_CPOL(0),/**< The inactive state value of SCK is low.  */
   SpiPolarity_InactiveHigh  = SPI_CTAR_CPOL(1),/**< The inactive state value of SCK is high. */
};

/**
 *  Clock Phase
 *  Selects which edge of SCK causes data to change and which edge causes data to be captured
 */
enum SpiPhase {
   SpiPhase_LeadingCapture = SPI_CTAR_CPHA(0),/**< Data is captured on the leading edge of SCK and changed on the following edge. */
   SpiPhase_LeadingChange  = SPI_CTAR_CPHA(1),/**< Data is changed on the leading edge of SCK and captured on the following edge */
};

/**
 * Bit transmission order (LSB/MSB first)
 */
enum SpiOrder {
   SpiOrder_MsbFirst = SPI_CTAR_LSBFE(0), /**< Transmit data LSB first */
   SpiOrder_LsbFirst = SPI_CTAR_LSBFE(1), /**< Transmit data MSB first */
};

/**
 * Calculate SPI.CTAR value from required frame-size
 *
 * @param frameSize Frame size i.e. number of bits to transfer
 *
 * @return Mask for CTAR.FMSZ
 */
uint32_t constexpr SpiFrameSize(unsigned frameSize) {
   return SPI_CTAR_FMSZ(frameSize-1);
}

/**
 * Transmit FIFO Fill Request interrupt/DMA enable (TFFF flag)
 */
enum SpiFifoTxRequest {
   SpiFifoTxRequest_Disabled  = SPI_RSER_TFFF_DIRS(0)|SPI_RSER_TFFF_RE(0),  // Requests disabled
   SpiFifoTxRequest_Interrupt = SPI_RSER_TFFF_DIRS(0)|SPI_RSER_TFFF_RE(1),  // Generate FIFO Fill Interrupt requests (TFFF flag)
   SpiFifoTxRequest_Dma       = SPI_RSER_TFFF_DIRS(1)|SPI_RSER_TFFF_RE(1),  // Generate FIFO Fill DMA requests (TFFF flag)
};

/**
 * Receive FIFO Drain Request interrupt/DMA enable (RFDF flag)
 */
enum SpiFifoRxRequest {
   SpiFifoRxRequest_Disabled  = SPI_RSER_RFDF_DIRS(0)|SPI_RSER_RFDF_RE(0),  // Requests disabled
   SpiFifoRxRequest_Interrupt = SPI_RSER_RFDF_DIRS(0)|SPI_RSER_RFDF_RE(1),  // Generate FIFO Drain Interrupt requests (RSER flag)
   SpiFifoRxRequest_Dma       = SPI_RSER_RFDF_DIRS(1)|SPI_RSER_RFDF_RE(1),  // Generate FIFO Drain DMA requests (RSER flag)
};

/**
 * Controls Transmit FIFO Underflow interrupts (TFUF flag)
 */
enum SpiFifoUnderflowInterrupt {
   SpiFifoUnderflowInterrupt_Disabled  = SPI_RSER_TFUF_RE(0),   // Transmit FIFO Underflow interrupts disabled
   SpiFifoUnderflowInterrupt_Enabled   = SPI_RSER_TFUF_RE(1),   // Transmit FIFO Underflow interrupts enabled (TFUF flag)
};

/**
 * Controls Receive FIFO Overflow interrupts (RFOF flag)
 */
enum SpiFifoOverflowInterrupt {
   SpiFifoOverflowInterrupt_Disabled  = SPI_RSER_RFOF_RE(0),   // Receive FIFO Overflow interrupts disabled
   SpiFifoOverflowInterrupt_Enabled   = SPI_RSER_RFOF_RE(1),   // Receive FIFO Overflow interrupts enabled (RFOF flag)
};

/**
 * Controls Transmit complete interrupts (TCF Flag)
 */
enum SpiTxCompleteInterrupt {
   SpiTxCompleteInterrupt_Disabled = SPI_RSER_TCF_RE(0),    // Transmission Complete Request Enable (TCF Flag)
   SpiTxCompleteInterrupt_Enabled  = SPI_RSER_TCF_RE(1),    // Transmission Complete Request Enable (TCF Flag)
};
/**
 * Controls DSPI Finished interrupts (EOQF flag)
 */
enum SpiEndOfQueueInterrupt {
   SpiEndOfQueueInterrupt_Disabled   = SPI_RSER_EOQF_RE(0),   // DSPI Finished Request Disabled
   SpiEndOfQueueInterrupt_Enabled    = SPI_RSER_EOQF_RE(1),   // DSPI Finished Request Enable (EOQF flag)
};
/**
 * Select which Peripheral Select Line to assert during transaction
 */
enum SpiPeripheralSelect {
   SpiPeripheralSelect_None = SPI_PUSHR_PCS(0),   //!< Select peripheral using programmatic GPIO
   SpiPeripheralSelect_0    = SPI_PUSHR_PCS(1<<0),//!< Select peripheral using SPI_PCS0 signal
   SpiPeripheralSelect_1    = SPI_PUSHR_PCS(1<<1),//!< Select peripheral using SPI_PCS1 signal
   SpiPeripheralSelect_2    = SPI_PUSHR_PCS(1<<2),//!< Select peripheral using SPI_PCS2 signal
   SpiPeripheralSelect_3    = SPI_PUSHR_PCS(1<<3),//!< Select peripheral using SPI_PCS3 signal
   SpiPeripheralSelect_4    = SPI_PUSHR_PCS(1<<4),//!< Select peripheral using SPI_PCS4 signal
};

/**
 * Select which CTAR to use for transaction
 */
enum SpiCtarSelect {
   SpiCtarSelect_0 = (0), //!< CTAR #0
   SpiCtarSelect_1 = (1), //!< CTAR #1
};

/**
 * Select whether Peripheral Select is returned to idle between transfers to the same peripheral
 */
enum SpiSelectMode {
   SpiSelectMode_Idle          = SPI_PUSHR_CONT(0), //!< Peripheral Select returns to idle between transfers
   SpiSelectMode_Continuous    = SPI_PUSHR_CONT(1), //!< Peripheral Select remains asserted between transfers
};

/**
 * Used to hold SPI configuration that may commonly be modified for different target peripherals
 */
struct SpiConfig {
   uint32_t pushr; //!<  PUSHR register value e.g. Target, selection mode etc
   uint32_t ctar;  //!<  CTAR register value e.g. Baud, number of bits, timing
};

/**
 * @brief Base class for representing an SPI interface
 */
class Spi {

protected:
   /** Callback to catch unhandled interrupt */
   static void unhandledCallback(uint32_t) {
      setAndCheckErrorCode(E_NO_HANDLER);
   }

public:

   const HardwarePtr<SPI_Type> spi;  //!< SPI hardware
   /**
    * Calculate communication speed factors for SPI
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  frequency      => Communication frequency in Hz
    *
    * @return CTAR register value including SPI_CTAR_BR, SPI_CTAR_PBR fields
    *
    * Note: Chooses the highest speed that is not greater than frequency.
    */
   static uint32_t calculateDividers(uint32_t clockFrequency, uint32_t frequency);

   /**
    * Calculate communication speed from SPI clock frequency and speed factors
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  clockFactors   => CTAR register value providing SPI_CTAR_BR, SPI_CTAR_PBR fields
    *
    * @return Clock frequency of SPI in Hz for these factors
    */
   static uint32_t calculateSpeed(uint32_t clockFrequency, uint32_t clockFactors);

   /**
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  frequency      => Communication frequency in Hz
    * @param[in]  cssck          => PCS assertion to SCK Delay Scaler
    * @param[in]  asc            => SCK to PCS negation delay
    * @param[in]  dt             => PCS negation to PCS assertion delay between transfers
    *
    * @return Combined masks for CTAR.BR, CTAR.PBR, CTAR.PCSSCK, CTAR.CSSCK, CTAR.PDT, CTAR.DT, CTAR.PCSSCK and CTAR.CSSCK
    */
   static uint32_t calculateCtar(uint32_t clockFrequency, uint32_t frequency, float cssck, float asc, float dt) {
      return calculateDividers(clockFrequency, frequency)|calculateDelays(clockFrequency, cssck, asc, dt);
   }

protected:
   uint32_t  pushrMask;            //!< Value to combine with data

   /**
    * Constructor
    *
    * @param[in]  baseAddress    Base address of SPI
    */
   Spi(uint32_t baseAddress) :
      spi(baseAddress), pushrMask(0) {
   }

   /**
    * Destructor
    */
   virtual ~Spi() {
   }

   /**
    * Calculate Delay factors
    * Used for ASC, DT and CSSCK
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  delay          => Desired delay in seconds
    * @param[in]  bestPrescale   => Best prescaler value (0=>/1, 1=>/3, 2=/5, 3=>/7)
    * @param[in]  bestDivider    => Best divider value (N=>/(2**(N+1)))
    *
    * @return true
    *
    * Note: Determines bestPrescaler and bestDivider for the smallest delay that is not less than delay.
    */
   static void calculateDelay(float clockFrequency, float delay, int &bestPrescale, int &bestDivider);

   /**
    * Calculate Delay factors for CSSCK (PCS assertion to SCK Delay Scaler)
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  delay          => Desired delay in seconds
    *
    * @return Combined masks for CTAR.PCSSCK and CTAR.CSSCK
    *
    * Note: Determines value for the smallest delay that is not less than delay.
    */
   static uint32_t calculateCSSCK(float clockFrequency, float delay) {
      int bestPrescale, bestDivider;
      calculateDelay(clockFrequency, delay, bestPrescale, bestDivider);
      return SPI_CTAR_PCSSCK(bestPrescale)|SPI_CTAR_CSSCK(bestDivider);
   }

   /**
    * Calculate Delay factors for ASC (SCK to PCS negation delay)
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  delay          => Desired delay in seconds
    *
    * @return Combined masks for CTAR.PASC and CTAR.ASC
    *
    * Note: Determines value for the smallest delay that is not less than delay.
    */
   static uint32_t calculateASC(float clockFrequency, float delay) {
      int bestPrescale, bestDivider;
      calculateDelay(clockFrequency, delay, bestPrescale, bestDivider);
      return SPI_CTAR_PASC(bestPrescale)|SPI_CTAR_ASC(bestDivider);
   }

   /**
    * Calculate Delay factors for DT (PCS negation to PCS assertion delay between transfers)
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  delay          => Desired delay in seconds
    *
    * @return Combined masks for CTAR.PDT and CTAR.DT
    *
    * Note: Determines value for the smallest delay that is not less than delay.
    */
   static uint32_t calculateDT(float clockFrequency, float delay) {
      int bestPrescale, bestDivider;
      calculateDelay(clockFrequency, delay, bestPrescale, bestDivider);
      return SPI_CTAR_PDT(bestPrescale)|SPI_CTAR_DT(bestDivider);
   }

   /**
    * Calculates the CTAR value for a given set of communication delays for SPI
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  cssck          => PCS assertion to SCK Delay Scaler
    * @param[in]  asc            => SCK to PCS negation delay
    * @param[in]  dt             => PCS negation to PCS assertion delay between transfers
    *
    * @return Combined masks for CTAR.PCSSCK, CTAR.CSSCK, CTAR.PDT, CTAR.DT, CTAR.PCSSCK and CTAR.CSSCK
    *
    * Note: Determines values for the smallest delay that is not less than specified delays.
    */
   static uint32_t calculateDelays(uint32_t clockFrequency, float cssck=1*USBDM::us, float asc=1*USBDM::us, float dt=1*USBDM::us) {
      uint32_t ctarValue;
      ctarValue  = calculateASC(clockFrequency, asc);
      ctarValue |= calculateDT(clockFrequency, dt);
      ctarValue |= calculateCSSCK(clockFrequency, cssck);
      return ctarValue;
   }

   /**
    * Sets Communication speed for SPI
    *
    * @param[in]  frequency      => Communication frequency in Hz
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  spiCtarSelect  => Index of CTAR register to modify
    *
    * Note: Chooses the highest speed that is not greater than frequency.
    */
   void setSpeed(uint32_t clockFrequency, uint32_t frequency, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) {
      spi->CTAR[spiCtarSelect] = (spi->CTAR[spiCtarSelect] & ~(SPI_CTAR_BR_MASK|SPI_CTAR_PBR_MASK)) | calculateDividers(clockFrequency, frequency);
   }

   /**
    * Sets the CTAR value for a given set of communication delays
    *
    * @param[in]  clockFrequency => Clock frequency of SPI in Hz
    * @param[in]  cssck          => PCS assertion to SCK Delay Scaler
    * @param[in]  asc            => SCK to PCS negation delay
    * @param[in]  dt             => PCS negation to PCS assertion delay between transfers
    * @param[in]  spiCtarSelect  => Index of CTAR register to modify
    *
    * Note: Determines values for the smallest delay that is not less than specified delays.
    */
   void setDelays(uint32_t clockFrequency, float cssck, float asc, float dt, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) {

      uint32_t ctarValue = spi->CTAR[spiCtarSelect] &
            ~(SPI_CTAR_ASC_MASK|SPI_CTAR_PASC_MASK|SPI_CTAR_DT_MASK|SPI_CTAR_PDT_MASK|SPI_CTAR_CSSCK_MASK|SPI_CTAR_PCSSCK_MASK);
      spi->CTAR[spiCtarSelect] = ctarValue|calculateDelays(clockFrequency, cssck, asc, dt);
   }

public:

#ifdef __CMSIS_RTOS
   /**
    * Obtain SPI mutex and set SPI configuration
    *
    * @param[in]  configuration  The configuration to set for the transaction
    * @param[in]  milliseconds   How long to wait in milliseconds. Use osWaitForever for indefinite wait
    *
    * @return osOK:                    The mutex has been obtain.
    * @return osErrorTimeoutResource:  The mutex could not be obtained in the given time.
    * @return osErrorResource:         The mutex could not be obtained when no timeout was specified.
    * @return osErrorParameter:        The parameter mutex_id is incorrect.
    * @return osErrorISR:              Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus startTransaction(SpiConfig &configuration, int milliseconds=osWaitForever) = 0;

   /**
    * Obtain SPI mutex (SPI configuration unchanged)
    *
    * @param[in]  milliseconds How long to wait in milliseconds. Use osWaitForever for indefinite wait.
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
    * Release SPI mutex
    *
    * @return osOK:              The mutex has been correctly released.
    * @return osErrorResource:   The mutex was not obtained before.
    * @return osErrorISR:        Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus endTransaction() = 0;
//#elif defined(__FREE_RTOS)
#else
   /**
    * Obtain SPI - dummy routine (non RTOS)
    */
   int startTransaction(int =0) {
      spi->MCR &= ~SPI_MCR_HALT_MASK;
      return 0;
   }
   /**
    * Obtain SPI and set SPI configuration
    *
    * @param[in] configuration The configuration values to set for the transaction.
    */
   int startTransaction(SpiConfig &configuration, int =0) {
      spi->MCR    &= ~SPI_MCR_HALT_MASK;
      setConfiguration(configuration);
      return 0;
   }
   /**
    * Release SPI - dummy routine (non RTOS)
    */
   int endTransaction() {
      spi->MCR  |= SPI_MCR_HALT_MASK;
      return 0;
   }
#endif

   /**
    * Enable pins used by SPI
    */
   virtual void enablePins() = 0;

   /**
    * Disable (restore to usual default) pins used by SPI
    */
   virtual void disablePins() = 0;

   /**
    * Sets the CTAR value for a given set of communication delays
    *
    * @param[in]  cssck          => PCS assertion to SCK Delay Scaler
    * @param[in]  asc            => SCK to PCS negation delay
    * @param[in]  dt             => PCS negation to PCS assertion delay between transfers
    * @param[in]  spiCtarSelect  => Index of CTAR register to modify
    *
    * Note: Determines values for the smallest delay that is not less than specified delays.
    */
   virtual void setDelays(float cssck=1*USBDM::us, float asc=1*USBDM::us, float dt=1*USBDM::us, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) = 0;

   /**
    * Sets the CTAR value for a given communication speed
    *
    * @param[in]  frequency => Frequency in Hz (0 => use default value)
    * @param[in]  spiCtarSelect   => Index of CTAR register to modify
    *
    * Note: Chooses the highest speed that is not greater than frequency.
    * Note: This will only have effect the next time a CTAR is changed
    */
   virtual void setSpeed(uint32_t frequency, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) = 0;

   /**
    * Sets Communication mode for SPI
    *
    * @param[in] spiMode   Controls clock polarity and the timing relationship between clock and data
    * @param[in] spiOrder  Bit transmission order (LSB/MSB first)
    * @param[in] spiCtarSelect => Index of CTAR register to modify
    */
   void setMode(SpiMode spiMode=SpiMode_0, SpiOrder spiOrder=SpiOrder_MsbFirst, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) {
      // Sets the default CTAR value with 8 bits
      spi->CTAR[spiCtarSelect] =
         (spiMode|spiOrder)|
         (spi->CTAR[spiCtarSelect]&~(SPI_CTAR_MODE_MASK|SPI_CTAR_LSBFE_MASK));
   }

   /**
    * Sets Communication mode for SPI
    *
    * @param[in] spiPolarity      Selects the inactive state of the Serial Communications Clock (SCK).
    * @param[in] spiPhase         Selects which edge of SCK causes data to change and which edge causes data to be captured
    * @param[in] spiOrder         Bit transmission order (LSB/MSB first)
    * @param[in] spiCtarSelect    Index of CTAR register to modify
    */
   void setMode(SpiPolarity spiPolarity, SpiPhase spiPhase, SpiOrder spiOrder=SpiOrder_MsbFirst, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) {
      // Sets the default CTAR value with 8 bits
      spi->CTAR[spiCtarSelect] =
         (spiPolarity|spiPhase|spiOrder)|
         (spi->CTAR[spiCtarSelect]&~(SPI_CTAR_MODE_MASK|SPI_CTAR_LSBFE_MASK));
   }

   /**
    * Sets Communication mode for SPI
    *
    * @param[in]  numBits => Number of bits in each transfer
    * @param[in]  spiCtarSelect => Index of CTAR register to modify
    */
   void setFrameSize(int numBits, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) {
      // Sets the frame size in CTAR
      spi->CTAR[spiCtarSelect] = (spi->CTAR[spiCtarSelect]&~(SPI_CTAR_FMSZ_MASK)) |
            SPI_CTAR_FMSZ(numBits-1);
   }
   
   /**
    * Sets up hardware peripheral select (SPI_PCSx) for transfer.
    * Also controls which CTAR is used for the transaction.
    *
    * @param[in]  spiPeripheralSelect  Which peripheral to select using SPI_PCSx signal
    * @param[in]  polarity             Polarity of SPI_PCSx, ActiveHigh or ActiveLow to select device
    * @param[in]  spiSelectMode        Whether SPI_PCSx signal is returned to idle between transfers
    * @param[in]  spiCtarSelect        Which CTAR to use for transaction
    */
   void setPeripheralSelect(
         SpiPeripheralSelect spiPeripheralSelect,
         Polarity            polarity,
         SpiSelectMode       spiSelectMode       = SpiSelectMode_Idle,
         SpiCtarSelect       spiCtarSelect       = SpiCtarSelect_0) {

      pushrMask = spiPeripheralSelect|spiSelectMode|SPI_PUSHR_CTAS(spiCtarSelect);

      if (polarity) {
         // ActiveHigh
         spi->MCR &= ~spiPeripheralSelect;
      }
      else {
         // ActiveLow
         spi->MCR |= spiPeripheralSelect;
      }
   }

   /**
    * Set the current SPI Selection mode.
    * This is used to change from the mode set by setPeripheralSelect() or startTransaction().
    *
    * Common usage:
    * - Configure the overall transaction to use SpiSelectMode_Continuous using setPeripheralSelect() or startTransaction().
    * - Do multiple txRx() operations.  The CS will remain selected _between_ operations.
    * - Change the mode to SpiSelectMode_Idle before the final operation using setPeripheralSelectMode().
    *   This will cause the active peripheral select to return to idle after the final operation.
    *
    *  @param[in]  spiSelectMode        Whether SPI_PCSx signal is returned to idle between transfers
    */
   void setPeripheralSelectMode(SpiSelectMode spiSelectMode) {
      pushrMask = (pushrMask&~SPI_PUSHR_CONT_MASK)|spiSelectMode;
   }

   /**
    *  Transmit and receive a series of values
    *
    *  @tparam T Type for data transfer (may be inferred from parameters)
    *
    *  @param[in]  dataSize  Number of values to transfer
    *  @param[in]  txData    Transmit bytes (may be nullptr for Receive only)
    *  @param[out] rxData    Receive byte buffer (may be nullptr for Transmit only)
    *
    *  @note: rxData may use same buffer as txData
    *  @note: Size of txData and rxData should be appropriate for transmission size.
    */
   template<typename T>
   void txRx(uint32_t dataSize, const T *txData, T *rxData=nullptr);

   /**
    * Transmit and receive a value over SPI using current settings
    *
    * @param[in] data Data to send (4-16 bits)
    *
    * @return Data received
    */
   uint16_t txRx(uint16_t data);

   /**
    * Transmit and receive a value over SPI
    *
    * @param[in] data - Data to send (4-16 bits) <br>
    *                   May include other control bits as for PUSHR
    *
    * @return Data received
    */
   uint32_t txRxRaw(uint32_t value);

   /**
    *  Set Configuration\n
    *  This includes timing settings, word length and transmit order\n
    *  Assumes the interface is already acquired through startTransaction
    *
    * @param[in]  configuration Configuration value
    */
   void setConfiguration(const SpiConfig &configuration) {
      spi->CTAR[0] = configuration.ctar;
      pushrMask    = configuration.pushr;
   }

   /**
    *  Get SPI configuration\n
    *  This includes timing settings, word length and transmit order
    *
    * @return Configuration value
    *
    * @note Typically used with startTransaction()
    */
   SpiConfig getConfiguration() {
      return SpiConfig{pushrMask,spi->CTAR[0]};
   }

   /**
    * Set SPI.CTAR0 value
    *
    * @param[in]  ctar 32-bit CTAR value
    */
   void setCTAR0Value(uint32_t ctar) {
      spi->CTAR[0] = ctar;
   }

   /**
    * Set SPI.CTAR1 value
    *
    * @param[in]  ctar 32-bit CTAR value
    */
   void setCTAR1Value(uint32_t ctar) {
      spi->CTAR[1] = ctar;
   }

   /**
    * Get SPI.CTAR0 value
    *
    * @return ctar 32-bit CTAR value
    */
   uint32_t getCTAR0Value() {
      return spi->CTAR[0];
   }

   /**
    * Get SPI.CTAR1 value
    *
    * @return ctar 32-bit CTAR value
    */
   uint32_t getCTAR1Value() {
      return spi->CTAR[1];
   }

   /**
    * Starts and stops the SPI transfers.
    *
    * @param[in] enable true to enable
    */
   void enableTransfer(bool enable=true) {
      if (enable) {
         spi->MCR &= ~SPI_MCR_HALT_MASK;
      }
      else {
         spi->MCR |= SPI_MCR_HALT_MASK;
      }
   }
   /**
    *
    * @param[in] spiFifoTxRequest   Transmit FIFO Fill Request interrupt/DMA enable (TFFF flag)
    * @param[in] spiFifoRxRequest   Receive FIFO Drain Request interrupt/DMA enable (RFDF flag)
    */
   void configureFifoRequests(
         SpiFifoTxRequest spiFifoTxRequest,
         SpiFifoRxRequest spiFifoRxRequest) {

      spi->RSER = (spi->RSER&~(SPI_RSER_TFFF_DIRS(1)|SPI_RSER_TFFF_RE(1)|SPI_RSER_RFDF_DIRS(1)|SPI_RSER_RFDF_RE(1)))|spiFifoTxRequest|spiFifoRxRequest;
   }
   /**
    *
    * @param[in] spiTxCompleteInterrupt      Controls Transmit complete interrupts (TCF Flag)
    * @param[in] spiEndOfQueueInterrupt      Controls DSPI Finished interrupts (EOQF flag)
    * @param[in] spiFifoUnderflowInterrupt   Controls Transmit FIFO Underflow interrupts (TFUF flag)
    * @param[in] spiFifoOverflowInterrupt    Controls Transmit FIFO Overflow interrupts (TFUF flag)
    */
   void configureInterrupts(
         SpiTxCompleteInterrupt     spiTxCompleteInterrupt     = SpiTxCompleteInterrupt_Disabled,
         SpiEndOfQueueInterrupt     spiEndOfQueueInterrupt     = SpiEndOfQueueInterrupt_Disabled,
         SpiFifoUnderflowInterrupt  spiFifoUnderflowInterrupt  = SpiFifoUnderflowInterrupt_Disabled,
         SpiFifoOverflowInterrupt   spiFifoOverflowInterrupt   = SpiFifoOverflowInterrupt_Disabled
         ) {

      spi->RSER = (spi->RSER&~(SPI_RSER_TFUF_RE(1)|SPI_RSER_RFOF_RE(1)|SPI_RSER_TCF_RE(1)|SPI_RSER_EOQF_RE(1)))|
            spiFifoUnderflowInterrupt|spiFifoOverflowInterrupt|spiTxCompleteInterrupt|spiEndOfQueueInterrupt;
   }
};

/**
 * @brief Template class representing a SPI interface
 *
 * @tparam  Info           Class describing Spi hardware
 */
template<class Info>
class SpiBase_T : public Spi {

public:
   /** Get reference to SPI hardware as struct */
   static volatile SPI_Type &spiPtr() { return Info::spi(); }

   /** Get base address of SPI hardware as uint32_t */
   static constexpr uint32_t spiBase() { return Info::baseAddress; }
   /** Get base address of SPI.MCR register as uint32_t */
   static constexpr uint32_t spiMCR() { return spiBase() + offsetof(SPI_Type, MCR); }
   /** Get base address of SPI.CR register as uint32_t */
   static constexpr uint32_t spiCR() { return spiBase() + offsetof(SPI_Type, TCR); }
   /** Get base address of SPI.CTAR[n] register as uint32_t */
   static constexpr uint32_t spiCTAR(unsigned index) { return spiBase() + offsetof(SPI_Type, CTAR[index]); }
   /** Get base address of SPI.SR register as uint32_t */
   static constexpr uint32_t spiSR() { return spiBase() + offsetof(SPI_Type, SR); }
   /** Get base address of SPI.PUSHR register as uint32_t */
   static constexpr uint32_t spiPUSHR() { return spiBase() + offsetof(SPI_Type, PUSHR); }
   /** Get base address of SPI.POPR register as uint32_t */
   static constexpr uint32_t spiPOPR() { return spiBase() + offsetof(SPI_Type, POPR); }

protected:
   /** Callback function for ISR */
   static SpiCallbackFunction sCallback;

public:
   /**
    * IRQ handler
    */
   static void irqHandler() {
      sCallback(SpiBase_T<Info>::getStatus());
   }

   /**
    * Set Callback function\n
    *
    *  @param[in]  callback  Callback function to be executed on interrupt.\n
    *                        Use nullptr to remove callback.
    */
   static __attribute__((always_inline)) void setCallback(SpiCallbackFunction callback) {
      usbdm_assert(Info::irqHandlerInstalled, "SPI not configure for interrupts");
      if (callback == nullptr) {
         callback = Spi::unhandledCallback;
      }
      sCallback = callback;
   }

#ifdef __CMSIS_RTOS
protected:
   /**
    * Mutex to protect access\n
    * Using a static accessor function avoids issues with static object initialisation order
    *
    * @return mutex
    */
   static CMSIS::Mutex &mutex() {
      /** Mutex to protect access - static so per SPI */
      static CMSIS::Mutex mutex;
      return mutex;
   }

public:
   /**
    * Obtain SPI mutex and set SPI configuration
    *
    * @param[in]  configuration  The configuration to set for the transaction
    * @param[in]  milliseconds   How long to wait in milliseconds. Use osWaitForever for indefinite wait
    *
    * @return osOK:                    The mutex has been obtain.
    * @return osErrorTimeoutResource:  The mutex could not be obtained in the given time.
    * @return osErrorResource:         The mutex could not be obtained when no timeout was specified.
    * @return osErrorParameter:        The parameter mutex_id is incorrect.
    * @return osErrorISR:              Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus startTransaction(SpiConfig &configuration, int milliseconds=osWaitForever) override {
      // Obtain mutex
      osStatus status = mutex().wait(milliseconds);
      if (status == osOK) {
         spi->MCR    &= ~SPI_MCR_HALT_MASK;
         // Change configuration for this transaction
         setConfiguration(configuration);
      }
      else {
         CMSIS::setAndCheckCmsisErrorCode(status);
      }
      return status;
   }

   /**
    * Obtain SPI mutex (SPI configuration unchanged)
    *
    * @param[in]  milliseconds How long to wait in milliseconds. Use osWaitForever for indefinite wait.
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
      if (status == osOK) {
         spi->MCR &= ~SPI_MCR_HALT_MASK;
      }
      else {
         CMSIS::setAndCheckCmsisErrorCode(status);
      }
      return status;
   }

   /**
    * Release SPI mutex
    *
    * @return osOK:              The mutex has been correctly released.
    * @return osErrorResource:   The mutex was not obtained before.
    * @return osErrorISR:        Cannot be called from interrupt service routines.
    *
    * @note The USBDM error code will also be set on error
    */
   virtual osStatus endTransaction() override {
      spi->MCR |= SPI_MCR_HALT_MASK;
      // Release mutex
      osStatus status = mutex().release();
      if (status != osOK) {
         CMSIS::setAndCheckCmsisErrorCode(status);
      }
      return status;
   }
#endif

public:
   /** SPI SCK (clock) Pin */
   using sckGpio  = GpioTable_T<Info, 0, ActiveHigh>;

   /** SPI SIN (data in = usually MISO) Pin */
   using sinGpio  = GpioTable_T<Info, 1, ActiveHigh>;

   /** SPI SOUT (data out = usually MOSI) Pin */
   using soutGpio = GpioTable_T<Info, 2, ActiveHigh>;

   /**
    * Configures all mapped pins associated with this peripheral
    */
   static void __attribute__((always_inline)) configureAllPins() {
      // Configure pins
      Info::initPCRs(PcrValue(PinPull_Up, PinDriveStrength_High,PinDriveMode_PushPull).value);
   }

   virtual void enablePins() override {
      configureAllPins();
   }

   virtual void disablePins() override {
      // Configure SPI pins to mux=0
      Info::clearPCRs();
   }

   /**
    * Sets Communication speed for SPI.
    * This also updates the communication delays based on the frequency.
    *
    * @param[in]  frequency      => Frequency in Hz (0 => use default value)
    * @param[in]  spiCtarSelect  => Index of CTAR register to modify
    *
    * Note: Chooses the highest speed that is not greater than frequency.
    */
   virtual void setSpeed(uint32_t frequency, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) override {
      Spi::setSpeed(Info::getClockFrequency(), frequency, spiCtarSelect);
      float SPI_PADDING2 = 1/(5.0*frequency);
      setDelays(SPI_PADDING2, SPI_PADDING2, SPI_PADDING2, spiCtarSelect);
   }

   /**
    * Sets the CTAR value for a given set of communication delays
    *
    * @param[in]  cssck          => PCS assertion to SCK Delay Scaler
    * @param[in]  asc            => SCK to PCS negation delay
    * @param[in]  dt             => PCS negation to PCS assertion delay between transfers
    * @param[in]  spiCtarSelect  => Index of CTAR register to modify
    *
    * Note: Determines values for the smallest delay that is not less than specified delays.
    */
   void setDelays(float cssck=1*USBDM::us, float asc=1*USBDM::us, float dt=1*USBDM::us, SpiCtarSelect spiCtarSelect=SpiCtarSelect_0) override {
      Spi::setDelays(Info::getClockFrequency(), cssck, asc, dt, spiCtarSelect);
   }

   /**
    * Calculates the timing aspects of a CTAR value based on frequency
    
    * @param[in]  frequency      => Communication frequency in Hz
    *
    * @return Combined masks for CTAR.BR, CTAR.PBR, CTAR.PCSSCK, CTAR.CSSCK, CTAR.PDT, CTAR.DT, CTAR.PCSSCK and CTAR.CSSCK
    */
   static uint32_t calculateCtar(uint32_t frequency) {
      float SPI_PADDING2 = 1/(5.0*frequency);
      return calculateDividers(Info::getClockFrequency(), frequency, SPI_PADDING2, SPI_PADDING2, SPI_PADDING2);
   }


   /**
    * Constructor
    */
   SpiBase_T() : Spi(Info::baseAddress) {

      // Check pin assignments
      static_assert(Info::info[Info::sckPin].gpioBit != UNMAPPED_PCR, "SPIx_SCK has not been assigned to a pin - Modify Configure.usbdm");
      static_assert(Info::info[Info::sinPin].gpioBit != UNMAPPED_PCR, "SPIx_SIN has not been assigned to a pin - Modify Configure.usbdm");
      static_assert(Info::info[Info::soutPin].gpioBit != UNMAPPED_PCR, "SPIx_SOUT has not been assigned to a pin - Modify Configure.usbdm");

      if (Info::mapPinsOnEnable) {
         configureAllPins();
      }

      // Enable SPI module clock
      Info::enableClock();
      __DMB();

      spi->MCR =
            SPI_MCR_HALT(1)|        // Halt transfers initially
            SPI_MCR_CLR_RXF(1)|     // Clear Rx FIFO
            SPI_MCR_CLR_TXF(1)|     // Clear Tx FIFO
            SPI_MCR_ROOE(1)|        // Receive FIFO Overflow Overwrite
            SPI_MCR_MSTR(1)|        // Master mode
            SPI_MCR_DCONF(0)|       // Must be zero
            SPI_MCR_SMPL_PT(0)|     // 0 system clocks between SCK edge and SIN sample
            SPI_MCR_PCSIS(0);       // Assume all SPI_PCSx active-high (initially)

      setCTAR0Value(0);         // Clear
      setCTAR1Value(0);         // Clear
      setFrameSize(8);          // Default 8-bit transfers
      setSpeed(Info::speed);    // Use default speed
      setMode((SpiMode)Info::mode, (SpiOrder)Info::lsbfe); // Use default mode and order
   }

   /**
    * Destructor
    */
   ~SpiBase_T() override {
   }

   /**
    * Gets and clears status flags.
    *
    * @return Status value (SPI->SR)
    */
   static uint32_t __attribute__((always_inline)) getStatus() {
      // Capture interrupt status
      uint32_t status = Info::spi->SR;
      // Clear captured flags
      Info::spi->SR = status;
      // Return status
      return status;
   }

};

/**
 *  Transmit and receive a series of values
 *
 *  @tparam T Type for data transfer (may be inferred from parameters)
 *
 *  @param[in]  dataSize  Number of values to transfer
 *  @param[in]  txData    Transmit bytes (may be nullptr for Receive only)
 *  @param[out] rxData    Receive byte buffer (may be nullptr for Transmit only)
 *
 *  @note: rxData may use same buffer as txData
 *  @note: Size of txData and rxData should be appropriate for transmission size.
 */
template<typename T>
void __attribute__((noinline)) Spi::txRx(uint32_t dataSize, const T *txData, T *rxData) {

   static_assert (((sizeof(T) == 1)||(sizeof(T) == 2)), "Size of data type T must be 8 or 16-bits");

   while(dataSize-->0) {
      uint32_t sendData = 0xFFFF;
      if (txData != nullptr) {
         sendData = (uint16_t)*txData++;
      }
      if (dataSize == 0) {
         // Mark last data
         sendData |= SPI_PUSHR_EOQ_MASK;
      }
      else {
         // Keep SPI_PCS asserted between data values
         sendData |= SPI_PUSHR_CONT_MASK;
      }
      spi->PUSHR = sendData|pushrMask;
      while ((spi->SR & SPI_SR_TCF_MASK)==0) {
      }
      spi->SR = SPI_SR_TCF_MASK|SPI_SR_EOQF_MASK;
      uint32_t receiveData = spi->POPR;
      if (rxData != nullptr) {
         *rxData++ = receiveData;
      }
   }
   // Wait until tx/rx complete
   while ((spi->SR&SPI_SR_TXRXS_MASK) == 0) {
      __asm__("nop");
   }
}

template<class Info> SpiCallbackFunction SpiBase_T<Info>::sCallback = Spi::unhandledCallback;

#if defined(USBDM_SPI0_IS_DEFINED)
/**
 * @brief Template class representing a SPI0 interface
 *
 * <b>Example</b>
 * @code
 * USBDM::Spi *spi = new USBDM::Spi0();
 *
 * uint8_t txData[] = {1,2,3};
 * uint8_t rxData[10];
 * spi->txRxBytes(sizeof(txData), txData, rxData);
 * @endcode
 *
 */
class Spi0 : public SpiBase_T<Spi0Info> {};
#endif

#if defined(USBDM_SPI1_IS_DEFINED)
/**
 * @brief Template class representing a SPI1 interface
 *
 * <b>Example</b>
 * @code
 * USBDM::Spi *spi = new USBDM::Spi1();
 *
 * uint8_t txData[] = {1,2,3};
 * uint8_t rxData[10];
 * spi->txRxBytes(sizeof(txData), txData, rxData);
 * @endcode
 *
 */
class Spi1 : public SpiBase_T<Spi1Info> {};

#endif
/**
 * End SPI_Group
 * @}
 */

#if defined(USBDM_SPI2_IS_DEFINED)
/**
 * @brief Template class representing a SPI2 interface
 *
 * <b>Example</b>
 * @code
 * USBDM::Spi *spi = new USBDM::Spi2();
 *
 * uint8_t txData[] = {1,2,3};
 * uint8_t rxData[10];
 * spi->txRxBytes(sizeof(txData), txData, rxData);
 * @endcode
 *
 */
class Spi2 : public SpiBase_T<Spi2Info> {};

#endif
/**
 * End SPI_Group
 * @}
 */

#if defined(USBDM_SPI3_IS_DEFINED)
/**
 * @brief Template class representing a SPI3 interface
 *
 * <b>Example</b>
 * @code
 * USBDM::Spi *spi = new USBDM::Spi3();
 *
 * uint8_t txData[] = {1,2,3};
 * uint8_t rxData[10];
 * spi->txRxBytes(sizeof(txData), txData, rxData);
 * @endcode
 *
 */
class Spi3 : public SpiBase_T<Spi3Info> {};

#endif
/**
 * End SPI_Group
 * @}
 */

} // End namespace USBDM

#endif /* INCLUDE_USBDM_SPI_H_ */
