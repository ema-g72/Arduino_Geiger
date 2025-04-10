/****************************************************************************
* File:     Geiger.cpp
* Project:  ArduinoGeiger
* Abstract: This class collects the events coming from the geiger tube and computes 
*           the count rate and dose rate. Every 3 seconds the number of events is 
*           added to a moving average.
* 
****************************************************************************/

#include <Arduino.h>
#include "Geiger.h"

// SBM-20 geiger tube dose ratio CPM to nanoSievert/hour: ((1/175.43) * 1000)
const float Geiger::CpmToNanoSvh = 5.7002793f;
// Number of elements in the moving average buffer.
const byte Geiger::MovingAverageBuffSize = 10;
// Time interval (msec) for the collection of one sample.
const uint16_t Geiger::SampleIntervalMs = 3000;

// ----------------------------------------------------------------------------
// Method: constructor
// ----------------------------------------------------------------------------
Geiger::Geiger() : mAverage(MovingAverageBuffSize)
{
}

// ----------------------------------------------------------------------------
// Method: resetCount()
// ----------------------------------------------------------------------------
void Geiger::resetCount()
{
   mTotalCount = 0;
   mIntervalCount = 0;
   mIntervalIndex = 1;
   mAverage.reset();
   mStartTimeMs = millis();
}

// ----------------------------------------------------------------------------
// Method: addCount(uint16_t num)
// ----------------------------------------------------------------------------
 void Geiger::addCount(uint16_t num)
 {
    mTotalCount += (unsigned long) num;

   unsigned long msec = (millis() - mStartTimeMs);

   // Insert count in moving average every "SampleIntervalMs" milliseconds.
   if( msec >= mIntervalIndex*SampleIntervalMs )
   {
      mAverage.add(mIntervalCount);

      mIntervalCount = num;
      mIntervalIndex ++;
   }
   else
   {
      mIntervalCount += num;
   }  
 }

// ----------------------------------------------------------------------------
// Method: getRateTimer(unsigned long *cpm, unsigned long *nsv_h)
// ----------------------------------------------------------------------------
void Geiger::getRateTimer(unsigned long *cpm, unsigned long *nsv_h)
{
   unsigned long sec = (millis() - mStartTimeMs)/1000;
   unsigned long ret = 0;

   // Convert to counts per minute.
   if( sec != 0 )
   {
      ret = (mTotalCount*60)/sec;
   }

   float tmp = ((float)ret) * CpmToNanoSvh;

   *cpm = ret;
   *nsv_h = (unsigned long) tmp;
}

// ----------------------------------------------------------------------------
// Method: getRateAverage(unsigned long *cpm, unsigned long *nsv_h)
// ----------------------------------------------------------------------------
void Geiger::getRateAverage(unsigned long *cpm, unsigned long *nsv_h)
{   
   byte num = max(1, mAverage.getNumberOfSamples());

   // Conversion factor from number of counts to counts per minute.
   float tmp = 60000.0/((float)SampleIntervalMs * num);

   tmp = tmp * mAverage.getAverage();
   *cpm = (unsigned long) tmp;

   tmp = tmp * CpmToNanoSvh;
   *nsv_h = (unsigned long) tmp;
}

