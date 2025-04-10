/****************************************************************************
* File:     LoopTimer.cpp
* Project:  Utilities
* Abstract: Defines a class for time expiration check, used to create timed loops.
* 
****************************************************************************/

#include <Arduino.h>
#include "LoopTimer.h"

// ----------------------------------------------------------------------------
// Method: constructor
// ----------------------------------------------------------------------------
LoopTimer::LoopTimer()
{
   mPrevMillis = millis();
}

// ----------------------------------------------------------------------------
// Method: forceExpired(int ms)
// ----------------------------------------------------------------------------
void LoopTimer::forceExpired(int ms)
{
   mPrevMillis -= ms;
}

// ----------------------------------------------------------------------------
// Method: expired(int ms)
// ----------------------------------------------------------------------------
boolean LoopTimer::expired(int ms)
{
   unsigned long now = millis();

   boolean ret = ((unsigned long)(now - mPrevMillis) >= (unsigned long)ms);
      
   if( ret )
   {
      mPrevMillis = now;
   }

   return ret;
}
