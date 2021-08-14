/*
 * Peripherals.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_PERIPHERALS_H_
#define SOURCES_PERIPHERALS_H_

#include <stdint.h>

#include "hardware.h"
#include "pit.h"
#include "ftm.h"
#include "cmp.h"

/// Channel 1 & 2 stand sensors
using Setbacks        = USBDM::GpioBField<17, 16, USBDM::ActiveLow>;

/// Channel 1 stand sensor
using Ch1Stand        = Setbacks::Bit<0>;

/// Channel 2 stand sensor
using Ch2Stand        = Setbacks::Bit<1>;

/// Resolution used for all ADC conversions.
constexpr USBDM::AdcResolution ADC_RESOLUTION = USBDM::AdcResolution_16bit_se;

/// Converter being used
using ADConverter = USBDM::Adc0;

/// High gain ADC channel
using HighGainAdcChannel = ADConverter::Channel<0>; // DP0/SE0

/// Low gain ADC channel
using LowGainAdcChanel = ADConverter::Channel<19>;  // DM0/SE19

/// Internal temperature sensor (25 - (Tvolts-0.719)/.001715)
using ChipTemperature    = ADConverter::Channel<0b11010>;

/// Internal band-gap (1.00V)
//using BandGap            = ADConverter::Channel<0b11011>;

/// External voltage reference for ADC (Vrefh)
constexpr float ADC_REF_VOLTAGE = 3.00;

/// Low gain op-amp feedback resistor
constexpr float LOW_GAIN_RF = 10000;

/// Low Gain op-amp input resistor 1
constexpr float LOW_GAIN_R1 = 10000;

/// Low Gain op-amp input resistor 2
constexpr float LOW_GAIN_R2 = 5100;

/// Low Gain op-amp input resistor R1//R2
constexpr float LOW_GAIN_R1_R2 = LOW_GAIN_R1*LOW_GAIN_R2/(LOW_GAIN_R1+LOW_GAIN_R2);

/// Low gain measurement ratio V/V i.e. converts ADC voltage to amplifier input voltage in V
/// Amplifier gain is 1 + (LOW_GAIN_RF/LOW_GAIN_RI)
constexpr float LOW_GAIN_MEASUREMENT_RATIO0   = LOW_GAIN_R1/(LOW_GAIN_R1+LOW_GAIN_RF);

/// Low gain measurement ratio V/V i.e. converts ADC voltage to amplifier input voltage in V
/// Amplifier gain is 1 + (LOW_GAIN_RF/LOW_GAIN_RI)
constexpr float LOW_GAIN_MEASUREMENT_RATIO1   = LOW_GAIN_R1_R2/(LOW_GAIN_R1_R2+LOW_GAIN_RF);

/// High gain op-amp feedback resistor
constexpr float HIGH_GAIN_RF = 56000;

/// High Gain op-amp input resistor
constexpr float HIGH_GAIN_RI = 1000;

/// High gain measurement ratio V/V i.e. converts ADC voltage to amplifier input voltage in V
/// Amplifier gain is 1 + (HIGH_GAIN_RF/HIGH_GAIN_RI)
constexpr float HIGH_GAIN_MEASUREMENT_RATIO0   = LOW_GAIN_MEASUREMENT_RATIO0*HIGH_GAIN_RI/(HIGH_GAIN_RI+HIGH_GAIN_RF);

/// High gain measurement ratio V/V i.e. converts ADC voltage to amplifier input voltage in V
/// Amplifier gain is 1 + (HIGH_GAIN_RF/HIGH_GAIN_RI)
constexpr float HIGH_GAIN_MEASUREMENT_RATIO1   = LOW_GAIN_MEASUREMENT_RATIO1*HIGH_GAIN_RI/(HIGH_GAIN_RI+HIGH_GAIN_RF);

/// Amplifier gain boost 0=>HIGH/LOW_GAIN_MEASUREMENT_RATIO0, 1=>HIGH/LOW_GAIN_MEASUREMENT_RATIO1
using Gain  = USBDM::GpioA<4, USBDM::ActiveHigh>;

/// Channel 1 Selected LED
using Ch1SelectedLed  = USBDM::GpioC<0, USBDM::ActiveHigh>;

/// Channel 2 Selected LED
using Ch2SelectedLed  = USBDM::GpioC<6, USBDM::ActiveHigh>;

/// Channel 1 Drive - 2-bit field
template<unsigned bit>
using Ch1Drive         = USBDM::GpioCField<2, 1, USBDM::ActiveLow>::Bit<bit>;

/// Channel 2 Drive - 2-bit field
template<unsigned bit>
using Ch2Drive         = USBDM::GpioCField<4, 3, USBDM::ActiveLow>::Bit<bit>;

/// Multiplexor selection and bias enable for ADC inputs (shared with ID inputs)
using AnalogueMultiplexor = USBDM::GpioDField<7, 4, USBDM::ActiveHigh>;

/// Bias enable on amplifier inputs
using BiasEnable = AnalogueMultiplexor::Bit<3>;

/// Gain boost enable on amplifiers
using HighGainEnable = AnalogueMultiplexor::Bit<0>;

/// ID1 input (shared with mux select 0 output) PTD.5
using Identify1 = ADConverter::Channel<6>;

/// ID2 input (shared with mux select 1 output) PTD.6
using Identify2 = ADConverter::Channel<7>;

/// Multiplexor selection for Thermocouple/NTC/ID
using AnalogueMultiplexorEnable  = USBDM::GpioC<5, USBDM::ActiveLow>;

/// Indicates use of Low gain ADC channel (indicates to software which ADC channel to use)
static constexpr uint8_t LOW_MASK = 0b00000000;

/// Indicates use of High gain ADC channel (indicates to software which ADC channel to use)
static constexpr uint8_t HIGH_MASK = 0b00010000;

/// Enables bias for measurement
static constexpr uint8_t BIAS_MASK = 0b00001000;

/// Enables extra gain for measurement
static constexpr uint8_t GAIN_MASK = 0b00000001;

/// Indicates which tool channel measurement is using
static constexpr uint8_t CHANNEL_MASK = 0b00000100;

/// Indicates channel 1 measurement
static constexpr uint8_t CH1_MASK = CHANNEL_MASK;

/// Indicates channel 2 measurement
static constexpr uint8_t CH2_MASK = 0b00000000;

/// Indicates sub-channel Xa measurement
static constexpr uint8_t CHA_MASK = 0b00000000;

/// Indicates sub-channel Xb measurement
static constexpr uint8_t CHB_MASK = 0b00000010;

/// Indicates use of ID1 ADC channel (indicates to software which ADC channel to use)
static constexpr uint8_t ID_MASK  = 0b00100000;

/**
 * Mux selection, and bias and gain enable for Amplifiers
 * Bottom 4-bits are directly written to hardware.
 * 5th bit is used to indicate which amplifier to use (software choice)
 */
enum MuxSelect : uint8_t {

   // Low gain amplifier measurement
   MuxSelect_ChaLowGain  = CHA_MASK,                                              /**< Channel A + Low gain amp */
   MuxSelect_ChbLowGain  = CHB_MASK,                                              /**< Channel B + Low gain amp */

   MuxSelect_Ch1aLowGain = CH1_MASK|CHA_MASK,                                     /**< Channel 1a + Low gain amp */
   MuxSelect_Ch1bLowGain = CH1_MASK|CHB_MASK,                                     /**< Channel 1b + Low gain amp */
   MuxSelect_Ch2aLowGain = CH2_MASK|CHA_MASK,                                     /**< Channel 2a + Low gain amp */
   MuxSelect_Ch2bLowGain = CH2_MASK|CHB_MASK,                                     /**< Channel 2b + Low gain amp */

   // High gain amplifier measurement
   MuxSelect_ChaHighGain  = MuxSelect_ChaLowGain|HIGH_MASK,                       /**< Channel A + High gain amp */
   MuxSelect_ChbHighGain  = MuxSelect_ChbLowGain|HIGH_MASK,                       /**< Channel B + High gain amp */

   MuxSelect_Ch1aHighGain = MuxSelect_Ch1aLowGain|HIGH_MASK,                      /**< Channel 1a + High gain amp */
   MuxSelect_Ch1bHighGain = MuxSelect_Ch1bLowGain|HIGH_MASK,                      /**< Channel 1b + High gain amp */
   MuxSelect_Ch2aHighGain = MuxSelect_Ch2aLowGain|HIGH_MASK,                      /**< Channel 2a + High gain amp */
   MuxSelect_Ch2bHighGain = MuxSelect_Ch2bLowGain|HIGH_MASK,                      /**< Channel 2b + High gain amp */

   // Low gain amplifier measurement + bias
   MuxSelect_ChaLowGainBiased  = MuxSelect_ChaLowGain|BIAS_MASK,                  /**< Channel A + Low gain amp + bias */
   MuxSelect_ChbLowGainBiased  = MuxSelect_ChbLowGain|BIAS_MASK,                  /**< Channel B + Low gain amp + bias */

   MuxSelect_Ch1aLowGainBiased = MuxSelect_Ch1aLowGain|BIAS_MASK,                 /**< Channel 1a + Low gain amp + bias */
   MuxSelect_Ch1bLowGainBiased = MuxSelect_Ch1bLowGain|BIAS_MASK,                 /**< Channel 1b + Low gain amp + bias */
   MuxSelect_Ch2aLowGainBiased = MuxSelect_Ch2aLowGain|BIAS_MASK,                 /**< Channel 2a + Low gain amp + bias */
   MuxSelect_Ch2bLowGainBiased = MuxSelect_Ch2bLowGain|BIAS_MASK,                 /**< Channel 2b + Low gain amp + bias */

   // High gain amplifier measurement + bias
   MuxSelect_ChaHighGainBiased  = MuxSelect_ChaHighGain|BIAS_MASK,                /**< Channel A + High gain amp + bias */
   MuxSelect_ChbHighGainBiased  = MuxSelect_ChbHighGain|BIAS_MASK,                /**< Channel B + High gain amp + bias */

   MuxSelect_Ch1aHighGainBiased = MuxSelect_Ch1aHighGain|BIAS_MASK,               /**< Channel 1a + High gain amp + bias */
   MuxSelect_Ch1bHighGainBiased = MuxSelect_Ch1bHighGain|BIAS_MASK,               /**< Channel 1b + High gain amp + bias */
   MuxSelect_Ch2aHighGainBiased = MuxSelect_Ch2aHighGain|BIAS_MASK,               /**< Channel 2a + High gain amp + bias */
   MuxSelect_Ch2bHighGainBiased = MuxSelect_Ch2bHighGain|BIAS_MASK,               /**< Channel 2b + High gain amp + bias */

   // Low gain amplifier measurement
   MuxSelect_ChaLowGainBoost  = MuxSelect_ChaLowGain|GAIN_MASK,                   /**< Channel A + Low gain amp + Boost */
   MuxSelect_ChbLowGainBoost  = MuxSelect_ChbLowGain|GAIN_MASK,                   /**< Channel B + Low gain amp + Boost */

   MuxSelect_Ch1aLowGainBoost = MuxSelect_Ch1aLowGain|GAIN_MASK,                  /**< Channel 1a + Low gain amp + Boost */
   MuxSelect_Ch1bLowGainBoost = MuxSelect_Ch1bLowGain|GAIN_MASK,                  /**< Channel 1b + Low gain amp + Boost */
   MuxSelect_Ch2aLowGainBoost = MuxSelect_Ch2aLowGain|GAIN_MASK,                  /**< Channel 2a + Low gain amp + Boost */
   MuxSelect_Ch2bLowGainBoost = MuxSelect_Ch2bLowGain|GAIN_MASK,                  /**< Channel 2b + Low gain amp + Boost */

   // High gain amplifier measurement
   MuxSelect_ChaHighGainBoost = MuxSelect_ChaHighGain|GAIN_MASK,                  /**< Channel A + High gain amp + Boost */
   MuxSelect_ChbHighGainBoost = MuxSelect_ChbHighGain|GAIN_MASK,                  /**< Channel B + High gain amp + Boost */

   MuxSelect_Ch1aHighGainBoost = MuxSelect_Ch1aHighGain|GAIN_MASK,                /**< Channel 1a + High gain amp + Boost */
   MuxSelect_Ch1bHighGainBoost = MuxSelect_Ch1bHighGain|GAIN_MASK,                /**< Channel 1b + High gain amp + Boost */
   MuxSelect_Ch2aHighGainBoost = MuxSelect_Ch2aHighGain|GAIN_MASK,                /**< Channel 2a + High gain amp + Boost */
   MuxSelect_Ch2bHighGainBoost = MuxSelect_Ch2bHighGain|GAIN_MASK,                /**< Channel 2b + High gain amp + Boost */

   // Low gain amplifier measurement + bias
   MuxSelect_ChaLowGainBoostBiased  = MuxSelect_Ch1aLowGainBiased|GAIN_MASK,      /**< Channel A + Low gain amp + Boost + bias */
   MuxSelect_ChbLowGainBoostBiased  = MuxSelect_Ch1bLowGainBiased|GAIN_MASK,      /**< Channel B + Low gain amp + Boost + bias */

   MuxSelect_Ch1aLowGainBoostBiased = MuxSelect_Ch1aLowGainBiased|GAIN_MASK,      /**< Channel 1a + Low gain amp + Boost + bias */
   MuxSelect_Ch1bLowGainBoostBiased = MuxSelect_Ch1bLowGainBiased|GAIN_MASK,      /**< Channel 1b + Low gain amp + Boost + bias */
   MuxSelect_Ch2aLowGainBoostBiased = MuxSelect_Ch2aLowGainBiased|GAIN_MASK,      /**< Channel 2a + Low gain amp + Boost + bias */
   MuxSelect_Ch2bLowGainBoostBiased = MuxSelect_Ch2bLowGainBiased|GAIN_MASK,      /**< Channel 2b + Low gain amp + Boost + bias */

   // High gain amplifier measurement + bias
   MuxSelect_ChaHighGainBoostBiased  = MuxSelect_ChaHighGainBiased|GAIN_MASK,     /**< Channel A + High gain amp + Boost + bias */
   MuxSelect_ChbHighGainBoostBiased  = MuxSelect_ChbHighGainBiased|GAIN_MASK,     /**< Channel B + High gain amp + Boost + bias */

   MuxSelect_Ch1aHighGainBoostBiased = MuxSelect_Ch1aHighGainBiased|GAIN_MASK,    /**< Channel 1a + High gain amp + Boost + bias */
   MuxSelect_Ch1bHighGainBoostBiased = MuxSelect_Ch1bHighGainBiased|GAIN_MASK,    /**< Channel 1b + High gain amp + Boost + bias */
   MuxSelect_Ch2aHighGainBoostBiased = MuxSelect_Ch2aHighGainBiased|GAIN_MASK,    /**< Channel 2a + High gain amp + Boost + bias */
   MuxSelect_Ch2bHighGainBoostBiased = MuxSelect_Ch2bHighGainBiased|GAIN_MASK,    /**< Channel 2b + High gain amp + Boost + bias */

   MuxSelect_Id  = ID_MASK,           /**< Convert on IDx ADC channel */

   MuxSelect_Id1 = ID_MASK|CH1_MASK,  /**< Convert on ID1 ADC channel */
   MuxSelect_Id2 = ID_MASK|CH2_MASK,  /**< Convert on ID2 ADC channel */

   MuxSelect_Complete = (uint8_t)-1
};

//struct MuxSelectValue {
//   union {
//      MuxSelect value;
//      struct {
//      unsigned fill:3;
//      unsigned highGain:1;
//      unsigned bias:1;
//      unsigned mux:2;
//      unsigned gainBoost:1;
//   };
//   };
//} __attribute__((__packed__));


/// PIT Channel to use for switch polling
using PollingTimerChannel = USBDM::Pit::Channel<0>;

/// PIT Channel to use for sample and control timing
using ControlTimerChannel = USBDM::Pit::Channel<1>;

/// Quadrature Encoder pins as a GPIO field
using QuadPhases = USBDM::GpioBField<1, 0, USBDM::ActiveLow>;

/// All buttons
using Buttons     = USBDM::GpioDField<3, 0, USBDM::ActiveLow>;

/// Quadrature Encoder shaft button
using QuadButton  = Buttons::Bit<0>;

/// Channel 2 Button
using Ch2Button   = Buttons::Bit<1>;

/// Channel 1 Button
using Ch1Button   = Buttons::Bit<2>;

/// Menu Button
using MenuButton   = Buttons::Bit<3>;

/// Zero Crossing Comparator
using ZeroCrossingComparator = USBDM::Cmp0;

/// Zero Crossing Detector Comparator input
constexpr USBDM::Cmp0Input ZeroCrossingInput = USBDM::Cmp0Input_Ptc7;

/// Over-current Comparator
using OverCurrentComparator = USBDM::Cmp1;

/// Over-current Detector Comparator input
constexpr USBDM::Cmp1Input OverCurrentInput = USBDM::Cmp1Input_VRefOut;

using GpioSpare1 = USBDM::GpioA<4,USBDM::ActiveHigh>;

using Debug      = GpioSpare1;

#endif /* SOURCES_PERIPHERALS_H_ */
