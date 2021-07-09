/*
 * Control.h
 *
 *  Created on: 5 Apr 2020
 *      Author: podonoghue
 */

#ifndef SOURCES_CONTROL_H_
#define SOURCES_CONTROL_H_

#include <PidController.h>
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

   /// Maximum duty cycle for tip drive
   ///      80% ~ 60W for 8 ohm element
   ///      90% ~ 65W for 8 ohm element
   /// Can't go above 90% as cycles are stolen for sampling
   static constexpr int      MAX_DUTY     = 80;

   /// Minimum duty cycle for tip drive 2% ~ 1.5W for 8 ohm element
   static constexpr int      MIN_DUTY     = 0;

   /// PID interval (1 rectified mains cycle)
   static constexpr float    PID_INTERVAL = 10*USBDM::ms;

private:
   /// Indicates the display needs updating
   bool     needRefresh = true;

   bool doReportPid       = false;
   bool doReportPidTitle  = false;

   /// Static this pointer for static members (call-backs)
   static Control *This;

   /// Moving window average for Chip temperature (internal MCU sensor)
   ChipTemperatureAverage chipTemperature;

   /// Counter to initiate PID reporting
   unsigned reportCount = 0;

   /// How often to refresh display
   /// Multiple of zero-crossing interval
   static constexpr unsigned REFRESH_INTERVAL = round(0.5/PID_INTERVAL);

   /// How often to log PID
   /// Multiple of zero-crossing interval
   static constexpr unsigned PID_LOG_INTERVAL = round(0.25/PID_INTERVAL);

public:
   /**
    * Constructor
    * Does minimal work - see initialise for main initialisation.
    */
   Control() {
      usbdm_assert(This == nullptr, "Control instantiated more than once");
      This = this;
   }

   float getChipTemperature() {
      return chipTemperature.getTemperature();
   }

   /**
    * Initialise the control
    */
   void initialise();

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
    * Change temperature of currently selected channel
    *
    * @param delta Amount to change by
    */
   void changeTemp(int16_t delta);

   /**
    * Interrupt handler for mains zero crossing Comparator
    * This uses the timer to schedule the switchOnHandler.
    * Occurs @100Hz or 120Hz ~ 10ms or 8.3ms
    */
   void zeroCrossingHandler();

   /**
    * Timer interrupt handler for updating PID
    */
   void pidHandler();

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
