/**
 * @file Control.h
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

private:
   /// Indicates the display needs updating
   bool needRefresh       = true;

   /// Used to hold off re-triggerring sequence until completed
   bool holdOff           = false;

   bool doReportPid       = false;
   bool doReportPidTitle  = false;

   /// Measurements to do near zero-crossing
   MuxSelect sequence[10];

   /// Number of measurements to do near zero-crossing (valid entries in sequence)
   unsigned sequenceIndex = 0;

   /// Moving window average for Chip temperature (internal MCU sensor)
   ChipTemperatureAverage chipTemperatureAverage;

   /// Counter to initiate PID reporting
   unsigned reportCount = 0;

   /// How often to refresh display
   /// Multiple of zero-crossing interval
   static constexpr unsigned REFRESH_INTERVAL = round(0.5/PID_INTERVAL);

   /// How often to log PID
   /// Multiple of zero-crossing interval
   static constexpr unsigned PID_LOG_INTERVAL = round(0.25/PID_INTERVAL);

   /// Idle time for display dimming (in milliseconds)
   unsigned idleTime = 0;

public:
   /**
    * Constructor
    * Does minimal work - see initialise for main initialisation.
    */
   Control() {}

   /**
    * Get chip temperature from on-chip sensor
    *
    * @return
    */
   float getChipTemperature() {
      return chipTemperatureAverage.getTemperature();
   }

   /**
    * Initialise the control
    */
   void initialise();

   /**
    * Toggle the enable state of a channel.
    *
    * @param chNum Channel to modify
    */
   void toggleEnable(unsigned chNum);

   /**
    * Enable channel.
    *
    * @param chNum Channel to enable
    */
   void enable(unsigned chNum);

   /**
    * Disable channel.
    * The other channel may become selected if enabled.
    *
    * @param chNum Channel to disable
    */
   void disable(unsigned chNum);

   /**
    * Change temperature of currently selected channel
    *
    * @param delta Amount to change by
    */
   void changeTemp(int16_t delta);

   /**
    * Comparator interrupt handler for controlling the heaters.
    * This is triggered just prior to the mains zero-crossing.
    * It also uses the timer to schedule the ADC sampling.
    */
   void zeroCrossingHandler();

   /**
    * Timer interrupt handler for updating PID
    * This includes the heater drives
    */
   void pidHandler();

   /**
    * Interrupt handler for ADC conversions
    *
    * @param [in] result     Conversion result from ADC channel
    * @param [in] adcChannel ADC channel providing the result
    *
    *   Initial conversion is started from zeroCrossingHandler().
    *   Several consecutive conversions are then chained in sequence.
    */
   void adcHandler(uint32_t result, int adcChannel);

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
    * Main event loop for front panel events
    */
   void eventLoop();

   /**
    * Checks if display is in use
    *
    * @param milliseconds Amount to increment the idle time by
    */
   void updateDisplayInUse(unsigned milliseconds);

   /**
    * Wakes up the display if off
    */
   void wakeUpDisplay();

   /**
    * Checks is the display is in use
    *
    * @return
    */
   bool isDisplayInUse();
};

extern Control control;

#endif /* SOURCES_CONTROL_H_ */
