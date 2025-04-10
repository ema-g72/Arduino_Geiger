/****************************************************************************
* File:     Geiger.h
* Project:  ArduinoGeiger
* Abstract: This class collects the events coming from the geiger tube and computes 
*           the count rate and dose rate. Every 3 seconds the number of events is 
*           added to a moving average.
*
****************************************************************************/

#ifndef _GEIGER_H_
#define _GEIGER_H_

#include <Arduino.h>
#include "MovingAverage.h"

// ----------------------------------------------------------------------------
// class: Geiger
// This class receives the event number through the method "addCount" and 
// considering the elapsed time computes the "Count per Minutes" and dose
// rate in microSievert per hour. The average value is computed in two ways:
// 1) moving average of last 30 seconds, updated every 3 seconds.
// 2) on an arbitrary time base starting from 'resetCount()' call.
// ----------------------------------------------------------------------------
class Geiger
{
public:

   // Constructor.
   Geiger();

   // Reset the time base and the moving average.
   void resetCount();

   // Add a number of events. This method shall be executed periodically
   // taking into account that the moving average is updated every 3 seconds.
   void addCount(uint16_t num);

   // Return the count rate from the total timer.
   void getRateTimer(uint32_t *cpm, uint32_t *nsv_h);

   // Return the count rate from the moving average.
   void getRateAverage(uint32_t *cpm, uint32_t *nsv_h);

protected:
   
   // Conversion factor from CPM to uSv/h (depends on geiger tube).
   static const float CpmToNanoSvh;
   // Number of elements in the moving average.
   static const byte MovingAverageBuffSize;
   // Every element of the moving average contains the number of counts collected in this time interval.
   static const uint16_t SampleIntervalMs;

   // Total number of counts from "mStartTimeMs".
   uint32_t mTotalCount;
   // Initial timer value in milliseconds.
   uint32_t mStartTimeMs;
   // Number of counts in a "SampleIntervalMs".
   uint16_t mIntervalCount;
   // Index used to increment the "SampleIntervalMs" time slots.
   uint32_t mIntervalIndex;
   // Moving average object.
   MovingAverage mAverage;
};

#endif
