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

namespace MyConstants {

using namespace USBDM;

/**
 * Control for channel voltage selection
 */
enum VoltageSelection {
   VoltageSelect_Off = 0b00, /**< Voltage Off */
   VoltageSelect_12V = 0b01, /**< Voltage = 12V */
   VoltageSelect_24V = 0b10, /**< Voltage = 24V */
};

/**
 * Control of channel drive outputs
 */
enum DriveSelection {
   DriveSelection_Off   = 0b00,                                     /**< Drive Off                        */
   DriveSelection_Left  = 0b01,                                     /**< Left drive only (A sub-channel)  */
   DriveSelection_Right = 0b10,                                     /**< Right drive only (B sub-channel) */
   DriveSelection_Both  = DriveSelection_Left|DriveSelection_Right, /**< Left+Right (A+B sub-channels)    */
};

/**
 * Used to OR together sub-channel outputs
 */
inline constexpr DriveSelection operator| (const DriveSelection &left, const DriveSelection &right) {
   return static_cast<DriveSelection>((unsigned)left|(unsigned)right);
}

/// Resolution used for all ADC conversions.
constexpr USBDM::AdcResolution ADC_RESOLUTION = USBDM::AdcResolution_16bit_se;

/// Vcc used as reference in some places (volts)
constexpr float VCC_REF_VOLTAGE = 3.30;

/// External voltage reference for ADC - VrefH (volts)
constexpr float ADC_REF_VOLTAGE = 3.00;

/// External voltage reference for CMP (Vdda)
constexpr float CMP_REF_VOLTAGE = VCC_REF_VOLTAGE;

/// Low gain op-amp feedback resistor
constexpr float LOW_GAIN_RF = 10000;

/// Low Gain op-amp input resistor
constexpr float LOW_GAIN_R1 = 10000;

/// Low Gain op-amp switched input resistor 1
constexpr float LOW_GAIN_R2 = 100;

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

enum ChannelNum    {ChannelNum_1 = 1,    ChannelNum_2 = 0, };
enum SubChannelNum {SubChannelNum_A = 0, SubChannelNum_B = 1, };

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
static inline constexpr uint8_t muxSelect(ChannelNum channelNum, SubChannelNum subChannelNum, bool bias, bool gainBoost) {

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
      (bias?BIAS_ON_MASK:BIAS_OFF_MASK) |
      (gainBoost?GAIN_BOOST_ON_MASK:GAIN_BOOST_OFF_MASK);
}

enum MuxSelect : uint8_t;

/**
 * Mux selection, and bias and gain enable for Amplifiers
 * Bottom 4-bits are directly written to hardware.
 * 5th bit is used to indicate which amplifier to use (software choice)
 */
enum MuxSelect : uint8_t {

   // Low gain amplifier measurement             Channel       Sub channel      Bias   Boost
   MuxSelect_Ch1aLowGain             = muxSelect(ChannelNum_1, SubChannelNum_A, false, false), /**< Channel 1a + Low gain amp */
   MuxSelect_Ch1bLowGain             = muxSelect(ChannelNum_1, SubChannelNum_B, false, false), /**< Channel 1b + Low gain amp */
   MuxSelect_Ch2aLowGain             = muxSelect(ChannelNum_2, SubChannelNum_A, false, false), /**< Channel 2a + Low gain amp */
   MuxSelect_Ch2bLowGain             = muxSelect(ChannelNum_2, SubChannelNum_B, false, false), /**< Channel 2b + Low gain amp */

   MuxSelect_ChaLowGain              = MuxSelect_Ch1aLowGain&~CHANNEL_MASK,                    /**< SubChannel A + Low gain amp */
   MuxSelect_ChbLowGain              = MuxSelect_Ch1bLowGain&~CHANNEL_MASK,                    /**< SubChannel B + Low gain amp */
   MuxSelect_LowGain                 = MuxSelect_Ch1bLowGain&~(CHANNEL_MASK|AB_MASK),          /**< Low gain amp */

   // Low gain amplifier measurement + bias      Channel       Sub channel      Bias   Boost
   MuxSelect_Ch1aLowGainBiased       = muxSelect(ChannelNum_1, SubChannelNum_A, true,  false), /**< Channel 1a + Low gain amp + bias */
   MuxSelect_Ch1bLowGainBiased       = muxSelect(ChannelNum_1, SubChannelNum_B, true,  false), /**< Channel 1b + Low gain amp + bias */
   MuxSelect_Ch2aLowGainBiased       = muxSelect(ChannelNum_2, SubChannelNum_A, true,  false), /**< Channel 2a + Low gain amp + bias */
   MuxSelect_Ch2bLowGainBiased       = muxSelect(ChannelNum_2, SubChannelNum_B, true,  false), /**< Channel 2b + Low gain amp + bias */

   MuxSelect_ChaLowGainBiased        = MuxSelect_Ch1aLowGainBiased&~CHANNEL_MASK,              /**< Channel A + Low gain amp + bias */
   MuxSelect_ChbLowGainBiased        = MuxSelect_Ch1bLowGainBiased&~CHANNEL_MASK,              /**< Channel B + Low gain amp + bias */
   MuxSelect_LowGainBiased           = MuxSelect_Ch1bLowGainBiased&~(CHANNEL_MASK|AB_MASK),    /**< Low gain amp + bias */

   // Low gain amplifier measurement             Channel       Sub channel      Bias   Boost
   MuxSelect_Ch1aHighGain            = muxSelect(ChannelNum_1, SubChannelNum_A, false, true),  /**< Channel 1a + Low gain amp + Boost */
   MuxSelect_Ch1bHighGain            = muxSelect(ChannelNum_1, SubChannelNum_B, false, true),  /**< Channel 1b + Low gain amp + Boost */
   MuxSelect_Ch2aHighGain            = muxSelect(ChannelNum_2, SubChannelNum_A, false, true),  /**< Channel 2a + Low gain amp + Boost */
   MuxSelect_Ch2bHighGain            = muxSelect(ChannelNum_2, SubChannelNum_B, false, true),  /**< Channel 2b + Low gain amp + Boost */

   MuxSelect_ChaHighGain             = MuxSelect_Ch1aHighGain&~CHANNEL_MASK,                   /**< Channel A + Low gain amp + Boost */
   MuxSelect_ChbHighGain             = MuxSelect_Ch1bHighGain&~CHANNEL_MASK,                   /**< Channel B + Low gain amp + Boost */
   MuxSelect_HighGain                = MuxSelect_Ch1bHighGain&~(CHANNEL_MASK|AB_MASK),         /**< Low gain amp + Boost */

   // Low gain amplifier measurement + bias      Channel       Sub channel      Bias   Boost
   MuxSelect_Ch1aHighGainBiased      = muxSelect(ChannelNum_1, SubChannelNum_A, true,  true),  /**< Channel 1a + Low gain amp + Boost + bias */
   MuxSelect_Ch1bHighGainBiased      = muxSelect(ChannelNum_1, SubChannelNum_B, true,  true),  /**< Channel 1b + Low gain amp + Boost + bias */
   MuxSelect_Ch2aHighGainBiased      = muxSelect(ChannelNum_2, SubChannelNum_A, true,  true),  /**< Channel 2a + Low gain amp + Boost + bias */
   MuxSelect_Ch2bHighGainBiased      = muxSelect(ChannelNum_2, SubChannelNum_B, true,  true),  /**< Channel 2b + Low gain amp + Boost + bias */

   MuxSelect_ChaHighGainBiased       = MuxSelect_Ch1aHighGainBiased&~CHANNEL_MASK,             /**< Channel A + Low gain amp + Boost + bias */
   MuxSelect_ChbHighGainBiased       = MuxSelect_Ch1bHighGainBiased&~CHANNEL_MASK,             /**< Channel B + Low gain amp + Boost + bias */
   MuxSelect_HighGainBiased          = MuxSelect_Ch1bHighGainBiased&~(CHANNEL_MASK|AB_MASK),   /**< Low gain amp + Boost + bias */

   MuxSelect_Idle                    = MuxSelect_Ch2bLowGain,

   MuxSelect_Complete = (uint8_t)-1,
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
   return static_cast<MuxSelect>(mux|((channelNum==ChannelNum_1)?CH1_MASK:CH2_MASK));
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
   return static_cast<MuxSelect>(mux|((subChannelNum==SubChannelNum_A)?CHA_MASK:CHB_MASK));
}

/// Maximum duty cycle for tip drive
///      80% ~ 60W for 8 ohm element
///      90% ~ 65W for 8 ohm element
/// WT-50 11 ohm element 100% ~52 watts
/// T12    8 ohm element 100% ~72 watts
/// WSP80  7 ohm element 100% ~82 watts
static constexpr int      MAX_DUTY     = 100;

/// Minimum duty cycle for tip drive 2% ~ 1.5W for 8 ohm element
static constexpr int      MIN_DUTY     = 0;

/**
 * Sample interval (1 cycle of the rectified mains)
 * Samples alternate between channels to reduce interaction
 */
static constexpr Seconds  SAMPLE_INTERVAL = 10.0_ms;

/**
 * Controller interval (2 cycles of the rectified mains)
 * Controller us update on every second cycle
 */
static constexpr Seconds  CONTROL_INTERVAL = 20.0_ms;

};

// A bit of a hack!
using namespace MyConstants;

#endif /* SOURCES_PERIPHERALS_H_ */
