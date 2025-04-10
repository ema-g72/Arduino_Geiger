/****************************************************************************
* File:     MovingAverage.h
* Project:  Utilities
* Abstract: Implements the moving average.
* 
****************************************************************************/

#ifndef _MOVING_AVERAGE_H_
#define _MOVING_AVERAGE_H_

#include <Arduino.h>

// ----------------------------------------------------------------------------
// class: MovingAverage
// This class implements a moving average based on the last (size) samples.
// ----------------------------------------------------------------------------
class MovingAverage
{
public:

   // Constructor, specify the buffer size.
   MovingAverage(byte size);

   // Destructor. This adds 100 bytes to the executable size, and it is never executed.   
   // ~MovingAverage();

   // Reset the internal buffer (samples history).
   void reset();

   // Add a new sample to the buffer.
   void add(uint16_t val);

   // Returns the current average value (multiplied by size).
   uint32_t getAverage();

   // Returns the current number of samples in the buffer (0 to size).
   byte getNumberOfSamples() { return mNumSamples; } 

protected:

   // Buffer with the last samples.
   uint16_t *mSamples;
   // Current sum of samples, average value = mNAverage/mNumSamples.
   uint32_t mNAverage;
   // Buffer size (number of elements).
   byte mSize;
   // Current number of samples.
   byte mNumSamples;
   // Current buffer index.
   byte mIndex;
};

#endif
