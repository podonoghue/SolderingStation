/**
 * @file     cmp.h (180.ARM_Peripherals/Project_Headers/cmp.h)
 * @brief    Analogue Comparator
 *
 * @version  V4.12.1.80
 * @date     13 April 2016
 */

#ifndef HEADER_CMP_H_
#define HEADER_CMP_H_
 /*
 * *****************************
 * *** DO NOT EDIT THIS FILE ***
 * *****************************
 *
 * This file is generated automatically.
 * Any manual changes will be lost.
 */
#include "derivative.h"
#include "pin_mapping.h"

namespace USBDM {

/**
 * @addtogroup CMP_Group CMP, Analogue Comparator
 * @brief Pins used for Analogue Comparator
 * @{
 */

/**
 * Filter Sample Count
 *
 * Represents the number of consecutive samples that must agree
 * prior to the comparator output filter accepting a new output state
 */
enum CmpFilterSamples {
   CmpFilterSamples_2    = CMP_CR0_FILTER_CNT(2), //!< 2 samples must agree
   CmpFilterSamples_3    = CMP_CR0_FILTER_CNT(3), //!< 3 samples must agree
   CmpFilterSamples_4    = CMP_CR0_FILTER_CNT(4), //!< 4 samples must agree
   CmpFilterSamples_5    = CMP_CR0_FILTER_CNT(5), //!< 5 samples must agree
   CmpFilterSamples_6    = CMP_CR0_FILTER_CNT(6), //!< 6 samples must agree
   CmpFilterSamples_7    = CMP_CR0_FILTER_CNT(7), //!< 7 samples must agree
};

/**
 * Determines hysteresis of comparator
 */
enum CmpHysteresis {
   CmpHysteresis_0 = CMP_CR0_HYSTCTR(0), //!< Minimum.
   CmpHysteresis_1 = CMP_CR0_HYSTCTR(1), //!< Middle-low
   CmpHysteresis_2 = CMP_CR0_HYSTCTR(2), //!< Middle-high
   CmpHysteresis_3 = CMP_CR0_HYSTCTR(3), //!< Maximum.
};

/**
 * Comparator interrupt selection
 */
enum CmpInterrupt {
   CmpInterrupt_None    = CMP_SCR_IER(0)|CMP_SCR_IEF(0),  //!< Neither edge
   CmpInterrupt_Rising  = CMP_SCR_IER(1)|CMP_SCR_IEF(0),  //!< Rising edge
   CmpInterrupt_Falling = CMP_SCR_IER(0)|CMP_SCR_IEF(1),  //!< Falling edge
   CmpInterrupt_Both    = CMP_SCR_IER(1)|CMP_SCR_IEF(1),  //!< Rising or falling edge
};

/**
 * Comparator event identification
 */
enum CmpEvent : uint8_t {
   CmpEvent_None    = CMP_SCR_CFR(0)|CMP_SCR_CFF(0),  //!< Neither edge
   CmpEvent_Rising  = CMP_SCR_CFR(1)|CMP_SCR_CFF(0),  //!< Rising edge
   CmpEvent_Falling = CMP_SCR_CFR(0)|CMP_SCR_CFF(1),  //!< Falling edge
   CmpEvent_Both    = CMP_SCR_CFR(1)|CMP_SCR_CFF(1),  //!< Rising or falling edge
};

/**
 * Used to represent the comparator status for interrupt handler
 */
struct CmpStatus {
   CmpEvent event;   //!< Event triggering handler
   uint8_t  state;   //!< State of CMPO at event

   constexpr CmpStatus(CmpEvent event, uint8_t  state) : event(event), state(state) {}
};

/**
 * Comparator mode
 */
enum CmpFilterClockSource {
   CmpFilterClockSource_Internal = CMP_CR1_SE(0),  //!< Internal filter clock
   CmpFilterClockSource_BusClock = CmpFilterClockSource_Internal, //!< Internal filter clock = Bus clock
   CmpFilterClockSource_External = CMP_CR1_SE(1),  //!< External filter clock
};

/**
 * Comparator mode
 */
enum CmpWindow {
   CmpWindow_Disabled = CMP_CR1_WE(0),  //!< Windowing disabled
   CmpWindow_Enabled  = CMP_CR1_WE(1),  //!< Windowing enabled
};

/**
 * Comparator power select
 */
enum CmpPower {
   CmpPower_LowPower    = CMP_CR1_PMODE(0), //!< Low power
   CmpPower_HighSpeed   = CMP_CR1_PMODE(1), //!< High speed
};

/**
 * Comparator invert
 */
enum CmpPolarity {
   CmpPolarity_Noninverted  = CMP_CR1_INV(0), //!< Not inverted
   CmpPolarity_Inverted     = CMP_CR1_INV(1), //!< Inverted
};

/**
 * Comparator output select
 */
enum CmpOutput {
   CmpOutput_Disabled  = CMP_CR1_OPE(0)|CMP_CR1_COS(0), //!< Disabled
   CmpOutput_Direct    = CMP_CR1_OPE(1)|CMP_CR1_COS(1), //!< Direct (unfiltered) output
   CmpOutput_Filtered  = CMP_CR1_OPE(1)|CMP_CR1_COS(0), //!< Filtered output
};

/**
 * Comparator mode
 */
enum CmpMode {
   CmpMode_Disabled    = CMP_CR1_EN(0),  //!< Disabled
   CmpMode_Enabled     = CMP_CR1_EN(1),  //!< Continuous
};

/**
 * DAC reference source
 */
enum CmpDacSource {
   CmpDacSource_Vin1 = CMP_DACCR_VRSEL(0), //!< Select Vrin1
   CmpDacSource_Vin2 = CMP_DACCR_VRSEL(1), //!< Select Vrin2
   CmpDacSource_Vdda = CmpDacSource_Vin2,  //!< Select Vrin2 = VddA
   CmpDacSource_Vref = CmpDacSource_Vin1,  //!< Select Vrin1 = Vref_out
};

/**
 * Type definition for CMP interrupt call back
 *
 * @param[in]  status Struct indicating interrupt source and state
 */
typedef void (*CmpCallbackFunction)(CmpStatus status);

/**
 * Template class representing a Analogue Comparator
 *
 * @tparam info      Information class for CMP
 *
 * @code
 * // Example using an interrupt handler on both rising and falling edges of comparator output
 *
 * // Comparator to use
 * using Cmp = Cmp0;
 *
 * // Comparator pin
 * using CmpPositiveInput = Cmp::Pin<4>;
 *
 * // Callback to handle comparator
 * void cmpCallback(CmpStatus status) {
 *    switch (status.event) {
 *       case CmpEvent_Rising  : console.write("Cmp rising  = ").writeln(Cmp::getCmpOutput()); break;
 *       case CmpEvent_Falling : console.write("Cmp falling = ").writeln(Cmp::getCmpOutput()); break;
 *       case CmpEvent_Both    : console.write("Cmp both    = ").writeln(Cmp::getCmpOutput()); break;
 *       case CmpEvent_None    : console.write("Cmp none    = ").writeln(Cmp::getCmpOutput()); break;
 *    }
 * }
 *
 * // Test comparator
 * void testComparator() {
 *    CmpPositiveInput::setInput();
 *
 *    Cmp::configure(CmpPower_HighSpeed, CmpHysteresis_2, CmpPolarity_Noninverted);
 *    Cmp::selectInputs(CmpPositiveInput::pinNum, CmpInput_DacRef);
 *    Cmp::setCallback(cmpCallback);
 *    Cmp::enableNvicInterrupts(NvicPriority_Normal);
 *    Cmp::enableInterrupts(CmpInterrupt_Both);
 *
 *    Cmp::configureDac(Cmp::MAXIMUM_DAC_VALUE/2, CmpDacSource_Vdda);
 *    for(;;) {
 *       // Sleep between interrupts
 *       Smc::enterWaitMode();
 *    }
 * }
 * @endcode
 */
template<class Info>
class CmpBase_T {

protected:
   /**
    * Limit index to permitted pin index range
    * Used to prevent noise from static assertion checks that detect a condition already detected in a more useful fashion.
    *
    * @param index   Index to limit
    *
    * @return Index limited to permitted range
    */
   static inline constexpr int limitIndex(int index) {
      if (index<0) {
         return 0;
      }
      if (index>(Info::numSignals-1)) {
         return Info::numSignals-1;
      }
      return index;
   }

   /** Class to static check output is mapped to a pin - Assumes existence */
   template<int cmpOutput> class CheckOutputIsMapped {

      // Check mapping - no need to check existence
      static constexpr bool Test1 = (Info::info[cmpOutput].gpioBit != UNMAPPED_PCR);

      static_assert(Test1, "CMP output is not mapped to a pin - Modify Configure.usbdm");

   public:
      /** Dummy function to allow convenient in-line checking */
      static constexpr void check() {}
   };

   /** Class to static check cmpInput exists and is mapped to a pin */
   template<int cmpInput> class CheckPinExistsAndIsMapped {
      // Tests are chained so only a single assertion can fail so as to reduce noise

      // Out of bounds value for function index
      static constexpr bool Test1 = (cmpInput>=0) && (cmpInput<(Info::numSignals));
      // Function is not currently mapped to a pin
      static constexpr bool Test2 = !Test1 || (Info::info[cmpInput].gpioBit != UNMAPPED_PCR);
      // Non-existent function and catch-all. (should be INVALID_PCR)
      static constexpr bool Test3 = !Test1 || !Test2 || (Info::info[cmpInput].gpioBit >= 0);

      static_assert(Test1, "Illegal CMP Input - Check Configure.usbdm for available inputs");
      static_assert(Test2, "CMP input is not mapped to a pin - Modify Configure.usbdm");
      static_assert(Test3, "CMP input doesn't exist in this device/package - Check Configure.usbdm for available input pins");

   public:
      /** Dummy function to allow convenient in-line checking */
      static constexpr void check() {}
   };

   /**
    * Callback to catch unhandled interrupt
    *
    * @param[in] status Struct indicating interrupt source and state
    */
   static void unhandledCallback(CmpStatus status) {
      (void)status;
      setAndCheckErrorCode(E_NO_HANDLER);
   }

   /** Callback function for ISR */
   static CmpCallbackFunction sCallback;

public:
   /// Pin mapped to CMP output
   using OutputPin = PcrTable_T<Info, Info::outputPin>;

   /**
    * Hardware instance pointer
    *
    * @return Reference to CMT hardware
    */
   static constexpr HardwarePtr<CMP_Type> cmp = Info::baseAddress;

   /**
    * IRQ handler
    */
   static void irqHandler() {
      unsigned status = cmp->SCR&(CMP_SCR_CFR_MASK|CMP_SCR_CFF_MASK|CMP_SCR_COUT_MASK);

      // Clear interrupt flags
      cmp->SCR = cmp->SCR | status;

      // Create status from snapshot
      CmpStatus cmpStatus{(CmpEvent)(status&(CMP_SCR_CFR_MASK|CMP_SCR_CFF_MASK)),(bool)(status&CMP_SCR_COUT_MASK)};

      // Call handler
      sCallback(cmpStatus);
   }

   /**
    * Wrapper to allow the use of a class member as a callback function
    * @note Only usable with static objects.
    *
    * @tparam T         Type of the object containing the callback member function
    * @tparam callback  Member function pointer
    * @tparam object    Object containing the member function
    *
    * @return  Pointer to a function suitable for the use as a callback
    *
    * @code
    * class AClass {
    * public:
    *    int y;
    *
    *    // Member function used as callback
    *    // This function must match CmpCallbackFunction
    *    void callback() {
    *       ...;
    *    }
    * };
    * ...
    * // Instance of class containing callback member function
    * static AClass aClass;
    * ...
    * // Wrap member function
    * auto fn = Cmp0::wrapCallback<AClass, &AClass::callback, aClass>();
    * // Use as callback
    * Cmp0::setCallback(fn);
    * @endcode
    */
   template<class T, void(T::*callback)(CmpStatus status), T &object>
   static CmpCallbackFunction wrapCallback() {
      static CmpCallbackFunction fn = [](CmpStatus status) {
         (object.*callback)(status);
      };
      return fn;
   }

   /**
    * Wrapper to allow the use of a class member as a callback function
    * @note There is a considerable space and time overhead to using this method
    *
    * @tparam T         Type of the object containing the callback member function
    * @tparam callback  Member function pointer
    * @tparam object    Object containing the member function
    *
    * @return  Pointer to a function suitable for the use as a callback
    *
    * @code
    * class AClass {
    * public:
    *    int y;
    *
    *    // Member function used as callback
    *    // This function must match CmpCallbackFunction
    *    void callback() {
    *       ...;
    *    }
    * };
    * ...
    * // Instance of class containing callback member function
    * AClass aClass;
    * ...
    * // Wrap member function
    * auto fn = Cmp0::wrapCallback<AClass, &AClass::callback>(aClass);
    * // Use as callback
    * Cmp0::setCallback(fn);
    * @endcode
    */
   template<class T, void(T::*callback)(CmpStatus status)>
   static CmpCallbackFunction wrapCallback(T &object) {
      static T &obj = object;
      static CmpCallbackFunction fn = [](CmpStatus status) {
         (obj.*callback)(status);
      };
      return fn;
   }

   /**
    * Set callback function
    *
    * @param[in] callback Callback function to execute on interrupt.\n
    *                     Use nullptr to remove callback.
    */
   static void setCallback(CmpCallbackFunction callback) {
      static_assert(Info::irqHandlerInstalled, "CMP not configured for interrupts");
      if (callback == nullptr) {
         callback = unhandledCallback;
      }
      sCallback = callback;
   }

public:
// Template _mapPinsOption.xml

   /**
    * Configures all mapped pins associated with CMP
    *
    * @note Locked pins will be unaffected
    */
   static void configureAllPins() {
   
      // Configure pins if selected and not already locked
      if constexpr (Info::mapPinsOnEnable && !(MapAllPinsOnStartup && (ForceLockedPins == PinLock_Locked))) {
         Info::initPCRs();
      }
   }

   /**
    * Disabled all mapped pins associated with CMP
    *
    * @note Only the lower 16-bits of the PCR registers are modified
    *
    * @note Locked pins will be unaffected
    */
   static void disableAllPins() {
   
      // Disable pins if selected and not already locked
      if constexpr (Info::mapPinsOnEnable && !(MapAllPinsOnStartup && (ForceLockedPins == PinLock_Locked))) {
         Info::clearPCRs();
      }
   }

   /**
    * Basic enable of CMP
    * Includes enabling clock and configuring all mapped pins if mapPinsOnEnable is selected in configuration
    */
   static void enable() {
      Info::enableClock();
      configureAllPins();
   }

   /**
    * Disables the clock to CMP and all mapped pins
    */
   static void disable() {
      disableNvicInterrupts();
      cmp->CR1 = CMP_CR1_EN(0);
      disableAllPins();
      Info::disableClock();
   }
// End Template _mapPinsOption.xml

   /**
    * Enable with default settings\n
    * Includes configuring all pins
    */
   static void defaultConfigure() {
      enable();

      // Initialise hardware
      cmp->CR0   = Info::cr0;
      cmp->CR1   = Info::cr1 | CMP_CR1_EN_MASK;
      cmp->FPR   = Info::fpr;
      cmp->SCR   = Info::scr;
      cmp->DACCR = Info::daccr;
      cmp->MUXCR = Info::muxcr;

      enableNvicInterrupts(Info::irqLevel);
   }

   /**
    * Configure all input pins associated with this CMP
    * The pins are set to analogue mode so no PCR settings are active.
    * This function is of use if mapAllPins and mapAllPinsOnEnable are not selected in USBDM configuration.
    */
   static void setInputs() {
      configureAllPins();
   }

   /**
    * Enable comparator output pin as output.\n
    * Pin control parameters default to values usually appropriate for the function being used.\n
    * The clock to the port will be enabled before changing the PCR.
    *
    * @param[in] pinDriveStrength One of PinDriveStrength_Low, PinDriveStrength_High
    * @param[in] pinDriveMode     One of PinDriveMode_PushPull, PinDriveMode_OpenDrain (defaults to PinPushPull)
    * @param[in] pinSlewRate      One of PinSlewRate_Slow, PinSlewRate_Fast (defaults to PinSlewRate_Fast)
    */
   static void setOutput(
         PinDriveStrength  pinDriveStrength  = OutputPin::defaultPcrValue,
         PinDriveMode      pinDriveMode      = OutputPin::defaultPcrValue,
         PinSlewRate       pinSlewRate       = OutputPin::defaultPcrValue
         ) {

      CheckOutputIsMapped<Info::outputPin>::check();

      // Enable CMP_OUT
      cmp->CR1 = cmp->CR1 | CMP_CR1_OPE_MASK;

      // Map CMP_OUT to pin
      OutputPin::setPCR(pinDriveStrength|pinDriveMode|pinSlewRate);
   }

   /*                                                             CmpFilterSamples cmpFilterSamplePeriod
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT   FPR.FILT_PER
    * 1  Disabled                              0      X      X      X                X
    * 2a/b Continuous                          1      0      0     (0     or         0)    COUT == COUTA
    * 3a   Sampled, Non-Filtered, external     1      0      1      1                X     COUTA combinational, COUT sampled by external clk pin
    * 3b   Sampled, Non-Filtered, internal     1      0      0      1               >=1    COUTA combinational, COUT sampled by busclk/PFR
    * 4a   Sampled, Filtered, external         1      0      1     >=2               X     COUTA combinational, COUT filtered by external clk pin
    * 4b   Sampled, Filtered, internal         1      0      0     >=2              >=1    COUTA combinational, COUT filtered by busclk/PFR
    * 5a/b Windowed                            1      1      0      0                0     COUT == COUTA clocked by bus clock when Window=1
    * 6    Windowed, Re-sampled                1      1      0      1               >=1    COUTA clocked by bus clock when Window=1, COUT re-sampled
    * 7    Windowed, Filtered                  1      1      0     >=2              >=1    COUTA clocked by bus clock when Window=1, COUT filtered by bus clock/PFR
    */

   /**
    * Base configuration - Continuous sampling: Modes 2a/2b.
    * Includes enabling clock and configuring all pins if mapPinsOnEnable is selected on configuration
    * DAC is initially disabled.
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 2a/b Continuous                          1      0      0      0              0     COUT == COUTA
    *
    * @param cmpPower        Power level. Tradeoff between speed and power consumption
    * @param cmpHysteresis   Hysteresis level for analogue comparator
    * @param cmpPolarity     Allows inversion of the comparator output
    */
   static void configure(
         CmpPower       cmpPower       = CmpPower_HighSpeed,
         CmpHysteresis  cmpHysteresis  = CmpHysteresis_2,
         CmpPolarity    cmpPolarity    = CmpPolarity_Noninverted
         ) {
      enable();

      // Initialise hardware
      cmp->CR1   = CmpMode_Enabled|cmpPower|cmpPolarity;
      cmp->CR0   = CMP_CR0_FILTER_CNT(0)|cmpHysteresis;
      cmp->FPR   = 0;
      cmp->SCR   = CMP_SCR_IER(0)|CMP_SCR_IEF(0);
      cmp->DACCR = (CMP_DACCR_VOSEL_MASK>>1)&CMP_DACCR_VOSEL_MASK;
      cmp->MUXCR = Info::muxcr;
   }

   /**
    * Set Continuous input - Mode 2a/b.
    * Assumes basic configuration done
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 2a/b Continuous                          1      0      0      0              0     COUT == COUTA
    */
   static void setInputContinous() {
      cmp->CR0 = (cmp->CR0&~CMP_CR0_FILTER_CNT_MASK)|CMP_CR0_FILTER_CNT(0);
      cmp->CR1 = (cmp->CR1&~(CMP_CR1_SE_MASK|CMP_CR1_WE_MASK))|CmpFilterClockSource_Internal|CmpWindow_Disabled;
      cmp->FPR = 0;
   }

   /**
    * Set Sampled, Non-Filtered input - Modes 3a/3b.
    *
    * Assumes basic configuration done
    * The TRGMUX is used as the CmpFilterClockSource_External source.
    *
    *                                                          CmpFilterSamples cmpFilterSamplePeriod
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 3a   Sampled, Non-Filtered, external     1      0      1      1              X     COUTA combinational, COUT sampled by external clk pin
    * 3b   Sampled, Non-Filtered, internal     1      0      0      1             >=1    COUTA combinational, COUT sampled by busclk/PFR
    *
    * @param[in] cmpFilterClockSource     Filter clock source
    * @param[in] cmpFilterSamplePeriod    Period of internal sample filter (1..255). \n
    *                                     Only applicable if CmpFilterClockSource=CmpFilterClockSource_Internal
    */
   static void setInputSampled(
         CmpFilterClockSource  cmpFilterClockSource,
         int                   cmpFilterSamplePeriod=1
         ) {
      usbdm_assert((cmpFilterClockSource == CmpFilterClockSource_External) || (cmpFilterSamplePeriod>=1), "Illegal parameters");
      cmp->CR0 = (cmp->CR0&~CMP_CR0_FILTER_CNT_MASK)|CMP_CR0_FILTER_CNT(1);
      cmp->CR1 = (cmp->CR1&~(CMP_CR1_SE_MASK|CMP_CR1_WE_MASK))|cmpFilterClockSource|CmpWindow_Disabled;
      cmp->FPR = cmpFilterSamplePeriod;
   }

   /**
    * Set Sampled, Filtered input - Modes 4a/4b.
    *
    * Assumes basic configuration done
    * The TRGMUX is used as the CmpFilterClockSource_External source.
    *
    *                                                          CmpFilterSamples cmpFilterSamplePeriod
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 4a   Sampled, Filtered, external         1      0      1     >=2             X     COUTA combinational, COUT filtered by external clk pin
    * 4b   Sampled, Filtered, internal         1      0      0     >=2            >=1    COUTA combinational, COUT filtered by busclk/PFR
    *
    * @param[in] cmpFilterSamples         Number samples that must agree before COUT changes (>=2)
    * @param[in] cmpFilterClockSource     Filter clock source
    * @param[in] cmpFilterSamplePeriod    Period of internal sample filter (1..255). \n
    *                                     Only applicable if CmpFilterClockSource_Internal
    */
   static void setInputFiltered(
         CmpFilterSamples      cmpFilterSamples,
         CmpFilterClockSource  cmpFilterClockSource=CmpFilterClockSource_Internal,
         uint8_t               cmpFilterSamplePeriod=1
         ) {
      usbdm_assert((cmpFilterClockSource == CmpFilterClockSource_External) || (cmpFilterSamplePeriod>=1), "Illegal parameters");
      usbdm_assert((cmpFilterSamples>1) , "Illegal parameter combination");
      cmp->CR0 = (cmp->CR0&~CMP_CR0_FILTER_CNT_MASK)|cmpFilterSamples;
      cmp->CR1 = (cmp->CR1&~(CMP_CR1_SE_MASK|CMP_CR1_WE_MASK))|cmpFilterClockSource|CmpWindow_Disabled;
      cmp->FPR = cmpFilterSamplePeriod;
   }

   /**
    * Set Windowed input - Modes 5a/5b.
    *
    * Assumes basic configuration done
    * The TRGMUX is used as the Window source.
    *
    *                                                          CmpFilterSamples cmpFilterSamplePeriod
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 5a/b Windowed                            1      1      0      0              0     COUT == COUTA clocked by bus clock when Window=1
    */
   static void setInputWindowed() {
      cmp->CR0 = (cmp->CR0&~CMP_CR0_FILTER_CNT_MASK)|0;
      cmp->CR1 = (cmp->CR1&~(CMP_CR1_SE_MASK|CMP_CR1_WE_MASK))|CmpFilterClockSource_Internal|CmpWindow_Enabled;
      cmp->FPR = 0;
   }

   /**
    * Set Windowed, Re-sampled input - mode 6.
    * Assumes basic configuration done.
    *                                                          CmpFilterSamples cmpFilterSamplePeriod
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 6    Windowed, Re-sampled                1      1      0      1             >=1    COUTA clocked by bus clock when Window=1, COUT re-sampled
    *
    * @param[in] cmpFilterSamplePeriod    Period of internal sample filter (1..255).
    */
   static void setInputWindowedResampled(
         int  cmpFilterSamplePeriod=1
         ) {
      usbdm_assert(cmpFilterSamplePeriod>=1, "Illegal parameters");
      cmp->CR0 = (cmp->CR0&~CMP_CR0_FILTER_CNT_MASK)|CMP_CR0_FILTER_CNT(1);
      cmp->CR1 = (cmp->CR1&~(CMP_CR1_SE_MASK|CMP_CR1_WE_MASK))|CmpFilterClockSource_Internal|CmpWindow_Enabled;
      cmp->FPR = cmpFilterSamplePeriod;
   }

   /**
    * Set Windowed, Filtered input - mode 7.
    * Assumes basic configuration done.
    *
    *                                                          CmpFilterSamples cmpFilterSamplePeriod
    *                                        CR1.EN CR1.WE CR1.SE CR0.FILTER_CNT FPR.FILT_PER
    * 7    Windowed, Filtered                  1      1      0     >=2            >=1    COUTA clocked by bus clock when Window=1, COUT filtered by bus clock/PFR
    *
    * @param[in] cmpFilterSamples         Number samples that must agree before COUT changes (>=2).
    * @param[in] cmpFilterSamplePeriod    Period of internal sample filter (1..255).
    */
   static void setInputWindowedFiltered(
         CmpFilterSamples     cmpFilterSamples,
         int                  cmpFilterSamplePeriod=1
         ) {
      usbdm_assert(cmpFilterSamplePeriod>=1, "Illegal parameters");
      cmp->CR0 = (cmp->CR0&~CMP_CR0_FILTER_CNT_MASK)|cmpFilterSamples;
      cmp->CR1 = (cmp->CR1&~(CMP_CR1_SE_MASK|CMP_CR1_WE_MASK))|CmpFilterClockSource_Internal|CmpWindow_Enabled;
      cmp->FPR = cmpFilterSamplePeriod;
   }

   /**
    * Enables Window mode
    * The TRGMUX is used as the Window source.
    *
    * @note Window mode may not be selected with external filter.
    */
   static void enableWindowMode() {
      cmp->CR1 = cmp->CR1 | CMP_CR1_WE_MASK;
   }

   /**
    * Disables Window mode
    */
   static void disableWindowMode() {
      cmp->CR1 = cmp->CR1 & ~CMP_CR1_WE_MASK;
   }

   /**
    * Enables Filter mode
    *
    * @note Window mode may not be selected with external filter.
    */
   static void enableFilterMode() {
      cmp->CR1 = cmp->CR1 | CMP_CR1_WE_MASK;
   }

   /**
    * Disables Filter mode
    */
   static void disableFilterMode() {
      cmp->CR1 = cmp->CR1 & ~CMP_CR1_WE_MASK;
   }

   /**
    * Get current output value of comparator
    *
    * @return true  => Cmp+ > Cmp- (unless inverted)
    * @return false => Cmp+ < Cmp- (unless inverted)
    */
   static bool getCmpOutput() {
      return cmp->SCR & CMP_SCR_COUT_MASK;
   }

   /**
    * Set input filtering and hysteresis
    *
    * @param[in] cmpFilterSamples Filtering clock pulses
    * @param[in] cmpHysteresis    Hysteresis level
    *
    * @note Window mode may not be selected with external filter.
    */
   static void setInputConditioning(CmpFilterSamples cmpFilterSamples, CmpHysteresis cmpHysteresis) {
      cmp->CR0 = cmpFilterSamples|cmpHysteresis;
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
    * Enable/disable edge interrupts
    *
    * @param[in]  cmpInterrupt Controls edge selection
    */
   static void enableInterrupts(CmpInterrupt cmpInterrupt) {
      cmp->SCR = (cmp->SCR&~(CMP_SCR_IER_MASK|CMP_SCR_IEF_MASK))|cmpInterrupt;
   }

   /**
    * Enable rising edge interrupts
    */
   static void enableRisingEdgeInterrupts() {
      cmp->SCR = cmp->SCR | CMP_SCR_IER_MASK;
   }

   /**
    * Disable rising edge interrupts
    */
   static void disableRisingEdgeInterrupts() {
      cmp->SCR = cmp->SCR & ~CMP_SCR_IER_MASK;
   }

   /**
    * Enable falling edge interrupts
    */
   static void enableFallingEdgeInterrupts() {
      cmp->SCR = cmp->SCR | CMP_SCR_IEF_MASK;
   }

   /**
    * Disable falling edge interrupts
    */
   static void disableFallingEdgeInterrupts() {
      cmp->SCR = cmp->SCR & ~CMP_SCR_IEF_MASK;
   }

#ifdef CMP_SCR_DMAEN_MASK
   /**
    * Enable DMA requests
    */
   static void enableDmaRequests() {
      cmp->SCR = cmp->SCR | CMP_SCR_DMAEN_MASK;
   }

   /**
    * Disable DMA requests
    */
   static void disableDmaRequests() {
      cmp->SCR = cmp->SCR & ~CMP_SCR_DMAEN_MASK;
   }
#endif

   /**
    * Clear edge interrupt flags
    */
   static void clearInterruptFlags() {
   cmp->SCR = cmp->SCR | CMP_SCR_CFR_MASK|CMP_SCR_CFF_MASK;
   }

   /**
    * Enable and configure DAC
    *
    * @param[in]  level        DAC level to select (0...MAXIMUM_DAC_VALUE) => (Vref/MAXIMUM_DAC_VALUE...Vref)
    * @param[in]  cmpDacSource Reference source select
    */
   static void configureDac(
         uint8_t       level,
         CmpDacSource  cmpDacSource) {
      cmp->DACCR = CMP_DACCR_DACEN_MASK|cmpDacSource|CMP_DACCR_VOSEL(level);
   }

   /**
    * Enable DAC
    */
   static void enableDAC() {
      cmp->DACCR = cmp->DACCR | CMP_DACCR_DACEN_MASK;
   }

   /**
    * Disable DAC
    */
   static void disableDAC() {
      cmp->DACCR = cmp->DACCR & ~CMP_DACCR_DACEN_MASK;
   }

   /** Maximum DAC value corresponding to Vref) */
   static constexpr int MAXIMUM_DAC_VALUE = CMP_DACCR_VOSEL_MASK;

   /**
    * Set DAC level\n
    * Assumes the DAC has already been configured by configureDac()
    *
    * @param[in]  level  DAC level to select (0...MAXIMUM_DAC_VALUE) => (Vref/MAXIMUM_DAC_VALUE...Vref)
    */
   static void setDacLevel(uint8_t level) {
      cmp->DACCR = (cmp->DACCR&~CMP_DACCR_VOSEL_MASK) | CMP_DACCR_VOSEL(level);
   }

protected:
   /**
    * Configure Comparator input sources
    *
    * @param[in]  positiveInput (0..7) (7 => DAC)
    * @param[in]  negativeInput (0..7) (7 => DAC)
    */
   static void selectInputs(unsigned positiveInput, unsigned negativeInput) {
      usbdm_assert((positiveInput<=7)&&(negativeInput<=7),"Illegal comparator input");

      //! MUX Control Register
      cmp->MUXCR =
         CMP_MUXCR_PSEL(positiveInput)| // Plus Input Mux Control
         CMP_MUXCR_MSEL(negativeInput); // Minus Input Mux Control
   }

   /**
    * Class representing a Comparator 0 pin
    *
    * @tparam cmpInput Number of comparator input (0-7) for associated pin.
    */
   template<typename T, T cmpInput>
   class PinBase_T : public PcrTable_T<Info, (unsigned)cmpInput> {

   public:
      // CmpInput number for use with selectInputs()
      static constexpr T pinNum = cmpInput;

      // Pin mask for use with Round Robin mode
      static constexpr uint8_t  pinMask = (1<<(unsigned)cmpInput);

      using Pcr = PcrTable_T<Info, (unsigned)cmpInput>;

      /**
       * Configure pin associated with CMP input.\n
       * This will map the pin to the CMP function. \n
       * The pins are set to analogue mode so no PCR settings are used.\n
       * The clock to the port will be enabled before changing the PCR.\n
       * This function is of use if mapAllPins and mapAllPinsOnEnable are not selected in USBDM configuration.
       *
       * @note Resets the entire Pin Control Register value (PCR value).
       */
      static void setInput() {

         CmpBase_T::CheckPinExistsAndIsMapped<(unsigned)cmpInput>::check();

         // Map pin
         Pcr::setPCR();
      }
   };
};

template<class Info> CmpCallbackFunction CmpBase_T<Info>::sCallback = CmpBase_T<Info>::unhandledCallback;

#if defined(USBDM_CMP_IS_DEFINED)
using Cmp = CmpBase_T<CmpInfo>;
#endif

#if defined(USBDM_CMP0_IS_DEFINED)
class Cmp0 : public CmpBase_T<Cmp0Info> {

public:
   /**
    * Select CMP0 inputs
    */
   enum Input {
      // Mapped inputs
      Input_0          = 0, //!< CMP0 input 0
      Input_1          = 1, //!< CMP0 input 1
      Input_2          = 2, //!< CMP0 input 2
      Input_3          = 3, //!< CMP0 input 3
      Input_4          = 4, //!< CMP0 input 4
      Input_5          = 5, //!< CMP0 input 5
      Input_6          = 6, //!< CMP0 input 6
      Input_7          = 7, //!< CMP0 input 7
      
      Input_Ptc7                = Input_1, ///< Mapped pin PTC7(p52)
      Input_ZeroCrossingInput   = Input_1, ///< Mapped pin PTC7(p52)
      Input_VrefOut             = Input_5, ///< Fixed pin  VREF_OUT(p17)
      Input_Bandgap             = Input_6, ///< Fixed pin  BANDGAP(Internal)
      Input_CmpDac              = Input_7, ///< Fixed pin  CMP_DAC(Internal)

   };

   /**
    * Configure Comparator input sources
    *
    * @param[in]  positiveInput (0..7) (7 => DAC)
    * @param[in]  negativeInput (0..7) (7 => DAC)
    */
   static __attribute__((always_inline)) void selectInputs(Input positiveInput, Input negativeInput) {
      CmpBase_T::selectInputs((unsigned)positiveInput, (unsigned)negativeInput);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(Input input, T &) {
      CmpBase_T::selectInputs((unsigned)input, (unsigned)T::pinNum);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(T &, Input input) {
      CmpBase_T::selectInputs((unsigned)T::pinNum, (unsigned)input);
   }

   template <class T1, class T2>
   static __attribute__((always_inline)) void selectInputs(T1 &, T2&) {
      CmpBase_T::selectInputs((unsigned)T1::pinNum, (unsigned)T2::pinNum);
   }

   /**
    * Class representing a Comparator pin
    *
    * @tparam cmpInput Number of comparator input (0-7) for associated pin.
    */
   template<int cmpInput>
   class Pin {
      using Pcr = PcrTable_T<Cmp0Info, (Input)cmpInput>;
   public:
      static constexpr Input pinNum = (Input)cmpInput;

      constexpr Pin() {}
      static void setInput() {
         Pcr::setPCR();
      }
   };
};
#endif

#if defined(USBDM_CMP1_IS_DEFINED)
class Cmp1 : public CmpBase_T<Cmp1Info> {

public:
   /**
    * Select CMP1 inputs
    */
   enum Input {
      // Mapped inputs
      Input_0          = 0, //!< CMP1 input 0
      Input_1          = 1, //!< CMP1 input 1
      Input_2          = 2, //!< CMP1 input 2
      Input_3          = 3, //!< CMP1 input 3
      Input_4          = 4, //!< CMP1 input 4
      Input_5          = 5, //!< CMP1 input 5
      Input_6          = 6, //!< CMP1 input 6
      Input_7          = 7, //!< CMP1 input 7
      
      Input_Adc0Se23            = Input_3, ///< Fixed pin  ADC0_SE23(p18)
      Input_VrefOut             = Input_5, ///< Fixed pin  VREF_OUT(p17)
      Input_Bandgap             = Input_6, ///< Fixed pin  BANDGAP(Internal)
      Input_CmpDac              = Input_7, ///< Fixed pin  CMP_DAC(Internal)

   };


   /**
    * Configure Comparator input sources
    *
    * @param[in]  positiveInput (0..7) (7 => DAC)
    * @param[in]  negativeInput (0..7) (7 => DAC)
    */
   static __attribute__((always_inline)) void selectInputs(Input positiveInput, Input negativeInput) {
      CmpBase_T::selectInputs((unsigned)positiveInput, (unsigned)negativeInput);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(Input input, T &) {
      CmpBase_T::selectInputs((unsigned)input, (unsigned)T::pinNum);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(T &, Input input) {
      CmpBase_T::selectInputs((unsigned)T::pinNum, (unsigned)input);
   }

   template <class T1, class T2>
   static __attribute__((always_inline)) void selectInputs(T1 &, T2&) {
      CmpBase_T::selectInputs((unsigned)T1::pinNum, (unsigned)T2::pinNum);
   }

   /**
    * Class representing a Comparator pin
    *
    * @tparam cmpInput Number of comparator input (0-7) for associated pin.
    */
   template<int cmpInput>
   class Pin {
      using Pcr = PcrTable_T<Cmp1Info, (Input)cmpInput>;
   public:
      static constexpr Input pinNum = (Input)cmpInput;

      constexpr Pin() {}
      static void setInput() {
         Pcr::setPCR();
      }
   };
};
#endif

#if defined(USBDM_CMP2_IS_DEFINED)
class Cmp2 : public CmpBase_T<Cmp2Info> {

public:
   /**
    * Select CMP2 inputs
    */
   enum Input {
      // Mapped inputs
      Input_0          = 0, //!< CMP2 input 0
      Input_1          = 1, //!< CMP2 input 1
      Input_2          = 2, //!< CMP2 input 2
      Input_3          = 3, //!< CMP2 input 3
      Input_4          = 4, //!< CMP2 input 4
      Input_5          = 5, //!< CMP2 input 5
      Input_6          = 6, //!< CMP2 input 6
      Input_7          = 7, //!< CMP2 input 7
      
      Input_Pta13               = Input_1, ///< Mapped pin PTA13(p29)
      Input_Overcurrent         = Input_1, ///< Mapped pin PTA13(p29)
      Input_Bandgap             = Input_6, ///< Fixed pin  BANDGAP(Internal)
      Input_CmpDac              = Input_7, ///< Fixed pin  CMP_DAC(Internal)

   };

   /**
    * Configure Comparator input sources
    *
    * @param[in]  positiveInput (0..7) (7 => DAC)
    * @param[in]  negativeInput (0..7) (7 => DAC)
    */
   static __attribute__((always_inline)) void selectInputs(Input positiveInput, Input negativeInput) {
      CmpBase_T::selectInputs((unsigned)positiveInput, (unsigned)negativeInput);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(Input input, T &) {
      CmpBase_T::selectInputs((unsigned)input, (unsigned)T::pinNum);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(T &, Input input) {
      CmpBase_T::selectInputs((unsigned)T::pinNum, (unsigned)input);
   }

   template <class T1, class T2>
   static __attribute__((always_inline)) void selectInputs(T1 &, T2&) {
      CmpBase_T::selectInputs((unsigned)T1::pinNum, (unsigned)T2::pinNum);
   }

   /**
    * Class representing a Comparator pin
    *
    * @tparam cmpInput Number of comparator input (0-7) for associated pin.
    */
   template<int cmpInput>
   class Pin {
      using Pcr = PcrTable_T<Cmp2Info, (Input)cmpInput>;
   public:
      static constexpr Input pinNum = (Input)cmpInput;

      constexpr Pin() {}
      static void setInput() {
         Pcr::setPCR();
      }
   };
};
#endif

#if defined(USBDM_CMP3_IS_DEFINED)
class Cmp3 : public CmpBase_T<Cmp3Info> {

public:
   /**
    * Select CMP3 inputs
    */
   enum Input {
      // Mapped inputs
      Input_0          = 0, //!< CMP3 input 0
      Input_1          = 1, //!< CMP3 input 1
      Input_2          = 2, //!< CMP3 input 2
      Input_3          = 3, //!< CMP3 input 3
      Input_4          = 4, //!< CMP3 input 4
      Input_5          = 5, //!< CMP3 input 5
      Input_6          = 6, //!< CMP3 input 6
      Input_7          = 7, //!< CMP3 input 7
      
// None Found
   };

   /**
    * Configure Comparator input sources
    *
    * @param[in]  positiveInput (0..7) (7 => DAC)
    * @param[in]  negativeInput (0..7) (7 => DAC)
    */
   static __attribute__((always_inline)) void selectInputs(Input positiveInput, Input negativeInput) {
      CmpBase_T::selectInputs((unsigned)positiveInput, (unsigned)negativeInput);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(Input input, T &) {
      CmpBase_T::selectInputs((unsigned)input, (unsigned)T::pinNum);
   }

   template <class T>
   static __attribute__((always_inline)) void selectInputs(T &, Input input) {
      CmpBase_T::selectInputs((unsigned)T::pinNum, (unsigned)input);
   }

   template <class T1, class T2>
   static __attribute__((always_inline)) void selectInputs(T1 &, T2&) {
      CmpBase_T::selectInputs((unsigned)T1::pinNum, (unsigned)T2::pinNum);
   }

   /**
    * Class representing a Comparator pin
    *
    * @tparam cmpInput Number of comparator input (0-7) for associated pin.
    */
   template<int cmpInput>
   class Pin {
      using Pcr = PcrTable_T<Cmp3Info, (Input)cmpInput>;
   public:
      static constexpr Input pinNum = (Input)cmpInput;

      constexpr Pin() {}
      static void setInput() {
         Pcr::setPCR();
      }
   };
};
#endif
/**
 * End CMP_Group
 * @}
 */
} // End namespace USBDM

#endif /* HEADER_CMP_H_ */
