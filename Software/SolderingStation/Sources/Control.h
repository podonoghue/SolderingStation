/*
 * Control.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CONTROL_H_
#define SOURCES_CONTROL_H_

#include "Pid.h"
#include "Peripherals.h"
#include "Display.h"
#include "Channel.h"
#include "DutyCycleCounter.h"
#include "MovingWindowAverage.h"

/**
 * This class implements most of the functionality including:
 *
 * - Main event loop with front panel update
 * - Interrupt driven monitoring of iron temperature and overload
 * - PWM drive of iron heaters
 */
class Control {

private:
   /// Maximum iron temperature for operation
   static constexpr int      MAX_TEMP     = 425;

   /// Minimum iron temperature for operation
   static constexpr int      MIN_TEMP     = 100;

   /// Delay between zero crossing and switching heaters on (us)
   static constexpr unsigned POWER_ON_DELAY  = 900;

   /// Delay between switching heater off and ADC conversions start (us)
   static constexpr unsigned SAMPLE_DELAY    = 4000;

   /// Delay between ADC conversions end and switching heaters off (us)
   static constexpr unsigned POWER_OFF_DELAY = 10000-SAMPLE_DELAY-1100;

   /// Indicates the display need updating
   bool     needRefresh = true;

   /// Mask indicating the ADC channels to be refreshed this cycle
   uint32_t adcChannelMask = 0;

   /// this pointer for static members (call-backs)
   static Control *This;

   /// Moving window average for Channel 1 tip temperature (thermocouple)
   ThermocoupleAverage<5> ch1TipTemperature;

   /// Moving window average for Channel 2 tip temperature (thermocouple)
   ThermocoupleAverage<5> ch2TipTemperature;

   /// Moving window average for Channel 1 cold junction temperature (NTC resistor)
   ThermistorAverage<5> ch1ColdJunctionTemperature;

   /// Moving window average for Channel 1 cold junction temperature (NTC resistor)
   ThermistorAverage<5> ch2ColdJunctionTemperature;

   /// Moving window average for Chip temperature (internal MCU sensor)
   ChipTemperatureAverage<5> chipTemperature;

   /// PWM duty-cyle counter for Channel 1
   DutyCycleCounter ch1DutyCycleCounter{100};

   /// PWM duty-cyle counter for Channel 2
   DutyCycleCounter ch2DutyCycleCounter{100};

   Pid ch1Pid{4.0, 0.0, 0.0, 10*USBDM::ms, 0.0, 100.0};
   Pid ch2Pid{4.0, 0.0, 0.0, 10*USBDM::ms, 0.0, 100.0};

   bool sequenceBusy = false;

public:
   /**
    * Constructor
    * Does minimal work - see initialise for main initialisation.
    */
   Control();

   /**
    * Initialise the control
    */
   void initialise();

   void testMenu();

   /**
    * Check if channel is enabled
    *
    * @param ch Channel to check
    */
   bool isEnabled(unsigned channel);

   /**
    * Toggle the enable state of a channel.
    * If becoming enabled it also becomes selected.
    * If becoming disabled the other channel may become selected if enabled.
    *
    * @param ch Channel to modify
    */
   void toggleEnable(unsigned channel);

   /**
    * Enable channel.
    * It also becomes selected.
    *
    * @param ch Channel to enable
    */
   void enable(unsigned ch);

   /**
    * Disable channel.
    * The other channel may become selected if enabled.
    *
    * @param ch Channel to disable
    */
   void disable(unsigned ch);

   /**
    * Backoff channel (if enabled).
    *
    * @param ch Channel to modify
    */
   void backOff(unsigned ch);

   /**
    * Wake-up channel (if in back-off).
    * It also becomes selected.
    *
    * @param ch Channel to modify
    */
   void wakeUp(unsigned ch);

   /**
    * Set the selected channel
    *
    * @param ch Channel to select
    */
   void setSelectedChannel(unsigned channel);

   /**
    * Change the temperature to the next preset value for the currently selected channel
    */
   void nextPreset();

   /**
    * Change temperature of currently selected channel
    *
    * @param delta Amount to change by
    */
   void changeTemp(int16_t delta);

   /**
    * Update the current preset from the current temperature of the currently selected channel
    */
   void updatePreset();

   /**
    * Interrupt handler for over current comparator
    */
   void overCurrentHandler();

   /**
    * Interrupt handler for mains zero crossing Comparator
    * This uses the timer to schedule the switchOnHandler.
    * Occurs @100Hz or 120Hz ~ 10ms or 8.3ms
    */
   void zeroCrossingHandler();

   /**
    * Timer interrupt handler for turning on heaters.
    * It also uses the timer to schedule the sampleHandler().
    */
   void switchOnHandler();

   /**
    * Timer interrupt handler for starting ADC sample sequence.
    * It also uses the timer to schedule the switchOffHandler().
    */
   void sampleHandler();

   /**
    * Timer interrupt handler for turning off the heaters.
    */
   void switchOffHandler();

   /**
    * Interrupt handler for ADC conversions
    *
    * @param[in] result  Conversion result from ADC channel
    * @param[in] channel ADC channel providing the result
    *
    *   Initial conversion is started from a timer call-back when a channel has an idle cycle.
    *   Several consecutive conversions are then chained in sequence.
    */
   void adcHandler(uint32_t result, int channel);

   /**
    * Refresh the display of channel information
    */
   void refresh();

   /**
    * Called to indicate the display needs update in the event loop.
    *
    * @note It does not do the update.
    */
   void setNeedsRefresh() {
      needRefresh = true;
   }

   /**
    * Debugging code
    */
   void reportChannel();

   /**
    * Event loop for front panel events.
    */
   void eventLoop();

};

#endif /* SOURCES_CONTROL_H_ */
