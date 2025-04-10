/****************************************************************************
* File:     MovingAverage.cpp
* Project:  Utilities
* Abstract: Implements the moving average.
* 
****************************************************************************/

#include "MovingAverage.h"

// ----------------------------------------------------------------------------
// Method: constructor
// ----------------------------------------------------------------------------
MovingAverage::MovingAverage(byte size)
{
   mSize = size;
   mSamples = (uint16_t *) malloc(size*2);
   reset();
}

// ----------------------------------------------------------------------------
// Method: destructor
// ----------------------------------------------------------------------------
/*
MovingAverage::~MovingAverage()
{
   if( mSamples != NULL )
   {
      free(mSamples);
   }   
}
*/

// ----------------------------------------------------------------------------
// Method: reset()
// ----------------------------------------------------------------------------
void MovingAverage::reset()
{
   mIndex = 0;
   mNumSamples = 0;
   mNAverage = 0;
}

// ----------------------------------------------------------------------------
// Method: add(uint16_t val)
// ----------------------------------------------------------------------------
void MovingAverage::add(uint16_t val)
{
   uint16_t oldestValue;

   if( mNumSamples < mSize )
   {
      oldestValue = 0;
      mNumSamples ++;
   }
   else
   {
      oldestValue =  mSamples[mIndex];
   }

   mSamples[mIndex] = val;

   mIndex ++;
   mIndex %= mSize;

   mNAverage += ((int32_t)val - (int32_t)oldestValue);
}

// ----------------------------------------------------------------------------
// Method: getAverage()
// ----------------------------------------------------------------------------
uint32_t MovingAverage::getAverage()
{
   return mNAverage;
}
