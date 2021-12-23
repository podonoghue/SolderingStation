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

/// Resolution used for all ADC conversions.
constexpr USBDM::AdcResolution ADC_RESOLUTION = USBDM::AdcResolution_16bit_se;

constexpr unsigned ADC_MAXIMUM = USBDM::FixedGainAdc::getSingleEndedMaximum(ADC_RESOLUTION);

/// External voltage reference for ADC (Vrefh)
constexpr float ADC_REF_VOLTAGE = 3.00;

/// External voltage reference for CMP (Vdda or VrefOut)
constexpr float CMP_REF_VOLTAGE = 3.30;

/// Low gain op-amp feedback resistor
constexpr float LOW_GAIN_RF = 10000;

/// Low Gain op-amp input resistor
constexpr float LOW_GAIN_R1 = 10000;

/// Low Gain op-amp switchable input resistor 1
constexpr float LOW_GAIN_R2 = 3300;

/// Pre-amplifier op-amp input resistor (R1//R2)
constexpr float LOW_GAIN_R1_R2 = (LOW_GAIN_R1*LOW_GAIN_R2)/(LOW_GAIN_R1+LOW_GAIN_R2);

/// Pre-amplifier measurement ratio V/V i.e. converts ADC voltage to amplifier input voltage in V
/// Amplifier gain is 1 + (LOW_GAIN_RF/LOW_GAIN_RI)
constexpr float LOW_GAIN_MEASUREMENT_RATIO_BOOST_OFF   = LOW_GAIN_R1/(LOW_GAIN_R1+LOW_GAIN_RF);

/// Pre-amplifier measurement ratio V/V i.e. converts ADC voltage to amplifier input voltage in V
/// Amplifier gain is 1 + (LOW_GAIN_RF/LOW_GAIN_RI)
constexpr float LOW_GAIN_MEASUREMENT_RATIO_BOOST_ON   = LOW_GAIN_R1_R2/(LOW_GAIN_R1_R2+LOW_GAIN_RF);

/// Bias resistors used for resistance measurement as voltage divider (switched 22K)
constexpr float BIAS_RESISTOR_VALUE = 22000;

/// Voltage used for bias resistor (Vdd ~ 3.3V)
constexpr float BIAS_VOLTAGE = 3.30;

/// Indicates gain boost (active low)
constexpr uint8_t GAIN_BOOST_MASK = 0b00000001;

/// Indicates which tool channel measurement is using
static constexpr uint8_t AB_MASK = 0b00000010;

/// Indicates sub-channel Xa measurement
static constexpr uint8_t CHA_MASK = 0b00000000;

/// Indicates sub-channel Xb measurement
static constexpr uint8_t CHB_MASK = AB_MASK;

/// Indicates which tool channel measurement is using
static constexpr uint8_t CHANNEL_MASK = 0b00000100;

/// Indicates channel 1 measurement
static constexpr uint8_t CH1_MASK = CHANNEL_MASK;

/// Indicates channel 2 measurement
static constexpr uint8_t CH2_MASK = 0b00000000;

/// Enables bias for measurement
static constexpr uint8_t BIAS_MASK = 0b00001000;

/// Indicates use of High/Low gain Amplifier path (indicates to software which ADC channel to use)
constexpr uint8_t AMPLIFIER_MASK = 0b00010000;

enum ChannelNum    {ChannelNum_1 = 1, ChannelNum_2 = 0, };
enum SubChannelNum {SubChannelNum_A = 0, SubChannelNum_B = 1, };
enum AmplifierNum  {AmplifierNum_LowGain = 0, AmplifierNum_ProgGain = 1, };

/**
 * Constructs mask to control ADC channel (amplifier used), gain boost and multiplexor selection
 *
 * @param channelNum       Chooses between 0/1 channel
 * @param subChannelNum    Chooses between A/B sub channels
 * @param amplifierNum     Chooses between high- and low-gain amplifier paths
 * @param bias             Enables bias during measurement
 * @param gainBoost        Enables extra gain during measurement
 *
 * @return  Mask encoding the above information (maps directly to hardware output
 */
static inline constexpr uint8_t muxSelect(ChannelNum channelNum, SubChannelNum subChannelNum, AmplifierNum amplifierNum, bool bias, bool gainBoost) {

   /// Indicates use of High gain ADC channel (indicates to software which ADC channel to use)
   constexpr uint8_t AMPLIFIER_HIGH_MASK = AMPLIFIER_MASK;

   /// Indicates use of Low gain ADC channel (indicates to software which ADC channel to use)
   constexpr uint8_t AMPLIFIER_LOW_MASK = 0b00000000;

   /// Enables bias for measurement
   constexpr uint8_t BIAS_ON_MASK = BIAS_MASK;

   /// Disables bias for measurement
   constexpr uint8_t BIAS_OFF_MASK = 0b00000000;

   /// Enables extra gain for measurement
   constexpr uint8_t GAIN_BOOST_ON_MASK = GAIN_BOOST_MASK;

   /// Disables extra gain for measurement
   constexpr uint8_t GAIN_BOOST_OFF_MASK = 0b00000000;

   return
      ((channelNum==ChannelNum_1)?CH1_MASK:CH2_MASK) |
      ((subChannelNum==SubChannelNum_A)?CHA_MASK:CHB_MASK) |
      ((amplifierNum==AmplifierNum_ProgGain)?AMPLIFIER_HIGH_MASK:AMPLIFIER_LOW_MASK) |
      (bias?BIAS_ON_MASK:BIAS_OFF_MASK) |
      (gainBoost?GAIN_BOOST_ON_MASK:GAIN_BOOST_OFF_MASK);
}

enum PgaGain {
   PgaGain_1 = USBDM::AdcPgaGain_1>>ADC_PGA_PGAG_SHIFT,
   PgaGain_2 = USBDM::AdcPgaGain_2>>ADC_PGA_PGAG_SHIFT,
   PgaGain_4 = USBDM::AdcPgaGain_4>>ADC_PGA_PGAG_SHIFT,
   PgaGain_8 = USBDM::AdcPgaGain_8>>ADC_PGA_PGAG_SHIFT,
   PgaGain_16 = USBDM::AdcPgaGain_16>>ADC_PGA_PGAG_SHIFT,
   PgaGain_32 = USBDM::AdcPgaGain_32>>ADC_PGA_PGAG_SHIFT,
   PgaGain_64 = USBDM::AdcPgaGain_64>>ADC_PGA_PGAG_SHIFT,
};

enum MuxSelect : uint8_t;

/**
 * Add PGA gain information to mask to control ADC channel (amplifier used), gain boost and multiplexor selection
 *
 * @param mux              Multiplexor value excluding PGA gain
 * @param gain             PGA gain (0-7)
 *
 * @return  Mask encoding the above information (maps directly to hardware output
 */
static inline constexpr uint8_t muxSelectAddPgaGain(uint8_t mux, PgaGain gain) {
   return
      static_cast<MuxSelect>(mux|((gain&0b111)<<5));
}

/**
 * Extract PGA gain from mux select value.
 *
 * This value may be used directly with Adc::configurePga()
 *
 * @param muxSelect
 */
static inline constexpr USBDM::AdcPgaGain muxPgaGain(MuxSelect muxSelect) {
   return static_cast<USBDM::AdcPgaGain>(ADC_PGA_PGAG(muxSelect>>8));
}

/**
 * Mux selection, and bias and gain enable for Amplifiers
 * Bottom 4-bits are directly written to hardware.
 * 5th bit is used to indicate which amplifier to use (software choice)
 */
enum MuxSelect : uint8_t {

   // Low gain amplifier measurement             Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aLowGain             = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_LowGain,  false, false), /**< Channel 1a + Low gain amp */
   MuxSelect_Ch1bLowGain             = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_LowGain,  false, false), /**< Channel 1b + Low gain amp */
   MuxSelect_Ch2aLowGain             = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_LowGain,  false, false), /**< Channel 2a + Low gain amp */
   MuxSelect_Ch2bLowGain             = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_LowGain,  false, false), /**< Channel 2b + Low gain amp */

   MuxSelect_ChaLowGain              = MuxSelect_Ch1aLowGain&~CHANNEL_MASK,                                           /**< SubChannel A + Low gain amp */
   MuxSelect_ChbLowGain              = MuxSelect_Ch1bLowGain&~CHANNEL_MASK,                                           /**< SubChannel B + Low gain amp */
   MuxSelect_LowGain                 = MuxSelect_Ch1bLowGain&~(CHANNEL_MASK|AB_MASK),                                 /**< Low gain amp */

   // High gain amplifier measurement            Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aProgGain            = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_ProgGain, false, false), /**< Channel 1a + High gain amp */
   MuxSelect_Ch1bProgGain            = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_ProgGain, false, false), /**< Channel 1b + High gain amp */
   MuxSelect_Ch2aProgGain            = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_ProgGain, false, false), /**< Channel 2a + High gain amp */
   MuxSelect_Ch2bProgGain            = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_ProgGain, false, false), /**< Channel 2b + High gain amp */

   MuxSelect_ChaProgGain             = MuxSelect_Ch1aProgGain&~CHANNEL_MASK,                                          /**< Channel A + High gain amp */
   MuxSelect_ChbProgGain             = MuxSelect_Ch1bProgGain&~CHANNEL_MASK,                                          /**< Channel B + High gain amp */
   MuxSelect_ProgGain                = MuxSelect_Ch1bProgGain&~(CHANNEL_MASK|AB_MASK),                                /**< High gain amp */

   // Low gain amplifier measurement + bias      Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aLowGainBiased       = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_LowGain,  true,  false), /**< Channel 1a + Low gain amp + bias */
   MuxSelect_Ch1bLowGainBiased       = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_LowGain,  true,  false), /**< Channel 1b + Low gain amp + bias */
   MuxSelect_Ch2aLowGainBiased       = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_LowGain,  true,  false), /**< Channel 2a + Low gain amp + bias */
   MuxSelect_Ch2bLowGainBiased       = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_LowGain,  true,  false), /**< Channel 2b + Low gain amp + bias */

   MuxSelect_ChaLowGainBiased        = MuxSelect_Ch1aLowGainBiased&~CHANNEL_MASK,                                     /**< Channel A + Low gain amp + bias */
   MuxSelect_ChbLowGainBiased        = MuxSelect_Ch1bLowGainBiased&~CHANNEL_MASK,                                     /**< Channel B + Low gain amp + bias */
   MuxSelect_LowGainBiased           = MuxSelect_Ch1bLowGainBiased&~(CHANNEL_MASK|AB_MASK),                           /**< Low gain amp + bias */

   // High gain amplifier measurement + bias     Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aProgGainBiased      = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_ProgGain, true,  false), /**< Channel 1a + High gain amp + bias */
   MuxSelect_Ch1bProgGainBiased      = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_ProgGain, true,  false), /**< Channel 1b + High gain amp + bias */
   MuxSelect_Ch2aProgGainBiased      = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_ProgGain, true,  false), /**< Channel 2a + High gain amp + bias */
   MuxSelect_Ch2bProgGainBiased      = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_ProgGain, true,  false), /**< Channel 2b + High gain amp + bias */

   MuxSelect_ChaProgGainBiased       = MuxSelect_Ch1aProgGainBiased&~CHANNEL_MASK,                                    /**< Channel A + High gain amp + bias */
   MuxSelect_ChbProgGainBiased       = MuxSelect_Ch1bProgGainBiased&~CHANNEL_MASK,                                    /**< Channel B + High gain amp + bias */
   MuxSelect_ProgGainBiased          = MuxSelect_Ch1bProgGainBiased&~(CHANNEL_MASK|AB_MASK),                          /**< High gain amp + bias */

   // Low gain amplifier measurement             Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aLowGainBoost        = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_LowGain,  false, true),  /**< Channel 1a + Low gain amp + Boost */
   MuxSelect_Ch1bLowGainBoost        = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_LowGain,  false, true),  /**< Channel 1b + Low gain amp + Boost */
   MuxSelect_Ch2aLowGainBoost        = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_LowGain,  false, true),  /**< Channel 2a + Low gain amp + Boost */
   MuxSelect_Ch2bLowGainBoost        = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_LowGain,  false, true),  /**< Channel 2b + Low gain amp + Boost */

   MuxSelect_ChaLowGainBoost         = MuxSelect_Ch1aLowGainBoost&~CHANNEL_MASK,                                      /**< Channel A + Low gain amp + Boost */
   MuxSelect_ChbLowGainBoost         = MuxSelect_Ch1bLowGainBoost&~CHANNEL_MASK,                                      /**< Channel B + Low gain amp + Boost */
   MuxSelect_LowGainBoost            = MuxSelect_Ch1bLowGainBoost&~(CHANNEL_MASK|AB_MASK),                            /**< Low gain amp + Boost */

   // High gain amplifier measurement            Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aProgGainBoost       = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_ProgGain, false, true),  /**< Channel 1a + High gain amp + Boost */
   MuxSelect_Ch1bProgGainBoost       = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_ProgGain, false, true),  /**< Channel 1b + High gain amp + Boost */
   MuxSelect_Ch2aProgGainBoost       = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_ProgGain, false, true),  /**< Channel 2a + High gain amp + Boost */
   MuxSelect_Ch2bProgGainBoost       = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_ProgGain, false, true),  /**< Channel 2b + High gain amp + Boost */

   MuxSelect_ChaProgGainBoost        = MuxSelect_Ch1aProgGainBoost&~CHANNEL_MASK,                                     /**< Channel A + High gain amp + Boost */
   MuxSelect_ChbProgGainBoost        = MuxSelect_Ch1bProgGainBoost&~CHANNEL_MASK,                                     /**< Channel B + High gain amp + Boost */
   MuxSelect_ProgGainBoost           = MuxSelect_Ch1bProgGainBoost&~(CHANNEL_MASK|AB_MASK),                           /**< High gain amp + Boost */

   // Low gain amplifier measurement + bias      Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aLowGainBoostBiased  = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_LowGain,  true,  true),  /**< Channel 1a + Low gain amp + Boost + bias */
   MuxSelect_Ch1bLowGainBoostBiased  = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_LowGain,  true,  true),  /**< Channel 1b + Low gain amp + Boost + bias */
   MuxSelect_Ch2aLowGainBoostBiased  = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_LowGain,  true,  true),  /**< Channel 2a + Low gain amp + Boost + bias */
   MuxSelect_Ch2bLowGainBoostBiased  = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_LowGain,  true,  true),  /**< Channel 2b + Low gain amp + Boost + bias */

   MuxSelect_ChaLowGainBoostBiased   = MuxSelect_Ch1aLowGainBoostBiased&~CHANNEL_MASK,                                /**< Channel A + Low gain amp + Boost + bias */
   MuxSelect_ChbLowGainBoostBiased   = MuxSelect_Ch1bLowGainBoostBiased&~CHANNEL_MASK,                                /**< Channel B + Low gain amp + Boost + bias */
   MuxSelect_LowGainBoostBiased      = MuxSelect_Ch1bLowGainBoostBiased&~(CHANNEL_MASK|AB_MASK),                      /**< Low gain amp + Boost + bias */

   // High gain amplifier measurement + bias     Channel       Sub channel      Amplifier path         Bias   Boost
   MuxSelect_Ch1aProgGainBoostBiased = muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_ProgGain, true,  true),  /**< Channel 1a + High gain amp + Boost + bias */
   MuxSelect_Ch1bProgGainBoostBiased = muxSelect(ChannelNum_1, SubChannelNum_B, AmplifierNum_ProgGain, true,  true),  /**< Channel 1b + High gain amp + Boost + bias */
   MuxSelect_Ch2aProgGainBoostBiased = muxSelect(ChannelNum_2, SubChannelNum_A, AmplifierNum_ProgGain, true,  true),  /**< Channel 2a + High gain amp + Boost + bias */
   MuxSelect_Ch2bProgGainBoostBiased = muxSelect(ChannelNum_2, SubChannelNum_B, AmplifierNum_ProgGain, true,  true),  /**< Channel 2b + High gain amp + Boost + bias */

   MuxSelect_ChaProgGainBoostBiased  = MuxSelect_Ch1aProgGainBoostBiased&~CHANNEL_MASK,                               /**< Channel A + High gain amp + Boost + bias */
   MuxSelect_ChbProgGainBoostBiased  = MuxSelect_Ch1bProgGainBoostBiased&~CHANNEL_MASK,                               /**< Channel B + High gain amp + Boost + bias */
   MuxSelect_ProgGainBoostBiased     = MuxSelect_Ch1bProgGainBoostBiased&~(CHANNEL_MASK|AB_MASK),                     /**< High gain amp + Boost + bias */

   // Identify tool by measuring ID resistor on A sub-channel
   MuxSelect_Identify                = MuxSelect_ChaLowGainBiased,  /**< Channel Xa + Low gain amp + Bias */

   MuxSelect_Idle                    = MuxSelect_Ch1aProgGainBoost,
   //muxSelect(ChannelNum_1, SubChannelNum_A, AmplifierNum_ProgGain,  false, true),  /**< Idle value - disable bias and gain boost */

   MuxSelect_Complete = (uint8_t)-1,

   // Various PGA settings with Boost
   MuxSelect_ProgGainBoostx1  = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_1),
   MuxSelect_ProgGainBoostx2  = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_2),
   MuxSelect_ProgGainBoostx4  = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_4),
   MuxSelect_ProgGainBoostx8  = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_8),
   MuxSelect_ProgGainBoostx16 = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_16),
   MuxSelect_ProgGainBoostx32 = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_32),
   MuxSelect_ProgGainBoostx64 = muxSelectAddPgaGain(MuxSelect_ProgGainBoost, PgaGain_64),
};

/**
 * Add channel information to mask to control ADC channel (amplifier used), gain boost and multiplexor selection
 *
 * @param mux              Multiplexor value excluding channel
 * @param channelNum       Chooses between 0/1 channel
 *
 * @return  Mask encoding the above information (maps directly to hardware output
 */
static inline constexpr MuxSelect muxSelectAddChannel(MuxSelect mux, ChannelNum channelNum) {
   return
      static_cast<MuxSelect>(mux|((channelNum==ChannelNum_1)?CH1_MASK:CH2_MASK));
}

/**
 * Add sub-channel information to mask to control ADC channel (amplifier used), gain boost and multiplexor selection
 *
 * @param mux              Multiplexor value excluding sub-channel
 * @param subChannelNum    Chooses between A/B sub channels
 *
 * @return  Mask encoding the above information (maps directly to hardware output
 */
static inline constexpr MuxSelect muxSelectAddSubChannel(MuxSelect mux, SubChannelNum subChannelNum) {
   return
      static_cast<MuxSelect>(mux|((subChannelNum==SubChannelNum_A)?CHA_MASK:CHB_MASK));
}

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

/// Maximum duty cycle for tip drive
///      80% ~ 60W for 8 ohm element
///      90% ~ 65W for 8 ohm element
/// WT-50 11 ohm element 100% ~52 watts
/// T12    8 ohm element 100% ~72 watts
/// WSP80  7 ohm element 100% ~82 watts
static constexpr int      MAX_DUTY     = 100;

/// Minimum duty cycle for tip drive 2% ~ 1.5W for 8 ohm element
static constexpr int      MIN_DUTY     = 0;

/// PID interval (1 rectified mains cycle)
static constexpr float    PID_INTERVAL = 10*USBDM::ms;

#endif /* SOURCES_PERIPHERALS_H_ */
