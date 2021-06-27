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
#include "Averaging.h"
#include "NonvolatileSettings.h"

class SettingsData;

/**
 * This class implements most of the functionality including:
 *
 * - Event loops with front panel update
 * - Interrupt driven monitoring of iron temperature and overload
 * - PWM drive of iron heaters
 */
class Control {

public:
   /// Maximum iron temperature for operation
   static constexpr int      MAX_TEMP     = 425;

   /// Minimum iron temperature for operation
   static constexpr int      MIN_TEMP     = 100;

   /// Maximum duty cycle for tip drive 80% ~ 60W for 8 ohms element
   /// Maximum duty cycle for tip drive 95% ~ 68W for 8 ohms element
   static constexpr int      MAX_DUTY     = 90;

   /// Minimum duty cycle for tip drive 2% ~ 1.5W for 8 ohms element
   static constexpr int      MIN_DUTY     = 0;

   /// PID interval in 10ms increments
   static constexpr int      PID_INTERVAL = 10;

private:
   /// Delay between zero crossing and switching heaters on (us)
   static constexpr unsigned POWER_ON_DELAY  = 200;

   /// Delay between switching heater off and ADC conversions start (us)
   static constexpr unsigned SAMPLE_DELAY    = 5000-POWER_ON_DELAY-100;

   /// Delay between ADC conversions start and switching heaters off (us)
   /// TODO - needs to change with main period
   static constexpr unsigned POWER_OFF_DELAY = 10000-SAMPLE_DELAY-200;

   /// Indicates the display needs updating
   bool     needRefresh = true;

   /// Indicates to sample temperatures on channel 1
   bool doSampleCh1 = false;

   /// Indicates to sample temperatures on channel 2
   bool doSampleCh2 = false;

   bool doReport       = false;
   bool doReportTitle  = false;

   /// Mask indicating which ADC channels to be refreshed this cycle
   uint32_t adcChannelMask = 0;

   /// Static this pointer for static members (call-backs)
   static Control *This;

   /// Moving window average for Chip temperature (internal MCU sensor)
   ChipTemperatureAverage chipTemperature;

   /// Debug - stop sequence restart when debugging
   bool sequenceBusy = false;

public:
   /**
    * Constructor
    * Does minimal work - see initialise for main initialisation.
    */
   Control() {
      usbdm_assert(This == nullptr, "Control instantiated more than once");
      This = this;
   }

   /**
    * Initialise the control
    */
   void initialise();

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
    * Set the selected channel
    *
    * @param ch Channel to select
    */
   void setSelectedChannel(unsigned channel);

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
    * Indicates the channel temperature or information has changed
    *
    * @return
    */
   bool needsRefresh() {
      return needRefresh;
   }

   /**
    * Debugging code
    */
   void reportChannel(Channel &ch);

   /**
    * Debugging code
    */
   void reportPid(Channel &ch);

   /**
    * Event loop for front panel events.
    */
   void eventLoop();
};

extern Control control;

#endif /* SOURCES_CONTROL_H_ */
