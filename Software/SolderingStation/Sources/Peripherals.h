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

/// Channel 1 stand sensor
//using Ch1Stand        = USBDM::GpioB<16, USBDM::ActiveLow>;
//using Ch1Stand        = USBDM::GpioB<17, USBDM::ActiveLow>;

/// Channel 2 stand sensor
//using Ch2Stand        = USBDM::GpioB<17, USBDM::ActiveLow>;

/// Channel 1 & 2 stand sensors
//using Setbacks        = USBDM::GpioBField<Ch2Stand::BITNUM, Ch1Stand::BITNUM, USBDM::ActiveLow>;

/// Over-current comparator
using Overcurrent     = USBDM::GpioD<7, USBDM::ActiveHigh>;

/// Resolution used for all ADC conversions.
constexpr USBDM::AdcResolution ADC_RESOLUTION = USBDM::AdcResolution_16bit_se;

/// Converter being used
using ADConverter = USBDM::Adc0;

/// Channel 1 tip thermocouple
using Ch1TipThermocouple = ADConverter::Channel<0>;   // DP0/SE0

/// Channel 2 tip thermocouple
using Ch2TipThermocouple = ADConverter::Channel<19>;  // DM0/SE19

/// Channel 1 handle thermocouple (cold junction)
using Ch1ColdJunctionNtc = ADConverter::Channel<15>;  // PTC1/SE15

/// Channel 2 handle thermocouple (cold junction)
using Ch2ColdJunctionNtc = ADConverter::Channel<14>;  // PTC0/SE14

/// Internal temperature sensor (25 - (Tvolts-0.719)/.001715)
using ChipTemperature    = ADConverter::Channel<0b11010>;

/// Internal band-gap (1.00V)
using BandGap            = ADConverter::Channel<0b11011>;

/// External voltage reference for ADC (Vrefh)
constexpr float ADC_REF_VOLTAGE = 3.00;

/// Thermocouple op-amp feedback resistor
constexpr float Rf = 100000;

/// Thermocouple op-amp input resistor
constexpr float Ri = 1000;

/// Thermocouple measurement ratio V/V i.e. converts ADC voltage to thermocouple voltage in V
/// Amplifier gain is Rf/Ri
constexpr float TC_MEASUREMENT_RATIO   = (Ri/Rf);

/// Channel 1 Selected LED
using Ch1SelectedLed  = USBDM::GpioC<6, USBDM::ActiveHigh>;

/// Channel 2 Selected LED
using Ch2SelectedLed  = USBDM::GpioD<6, USBDM::ActiveHigh>;

/// Channel 1 Drive
using Ch1Drive        = USBDM::GpioC<3, USBDM::ActiveLow>;

/// Channel 2 Drive
using Ch2Drive        = USBDM::GpioC<4, USBDM::ActiveLow>;

/// Channel 1 ID pin and bias resistor share a pin
using Ch1Id           = ADConverter::Channel<4>; // V3 only
//using Ch1BiasResistor = Ch1Id::GpioPin<USBDM::ActiveHigh>;
using Ch1BiasResistor = USBDM::GpioB<17, USBDM::ActiveLow>; // hack for V2

/// Channel 2 ID pin and bias resistor share a pin
using Ch2Id           = ADConverter::Channel<5>; // V3 only
using Ch2BiasResistor = Ch2Id::GpioPin<USBDM::ActiveHigh>;

/// PIT Channel to use for switch polling
using PollingTimerChannel = USBDM::Pit::Channel<0>;

/// PIT Channel to use for sample and control timing
using ControlTimerChannel = USBDM::Pit::Channel<1>;

/// Quadrature Encoder pins as a GPIO field
using QuadPhases = USBDM::GpioBField<1, 0, USBDM::ActiveLow>;

/// All buttons
using Buttons     = USBDM::GpioDField<5, 2, USBDM::ActiveLow>;

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

using GpioSpare1 = USBDM::GpioA<4,USBDM::ActiveHigh>;
using GpioSpare2 = USBDM::GpioC<2,USBDM::ActiveHigh>; // V2 only
using GpioSpare3 = USBDM::GpioD<1,USBDM::ActiveHigh>; // V2 only

using Debug        = USBDM::GpioB<16, USBDM::ActiveLow>;

#endif /* SOURCES_PERIPHERALS_H_ */
