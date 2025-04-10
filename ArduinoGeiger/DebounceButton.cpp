/****************************************************************************
* File:     DebounceButton.cpp
* Project:  Utilities
* Abstract: Implements a software debouncer for mechanical buttons.
*
****************************************************************************/

#include "DebounceButton.h"

// ----------------------------------------------------------------------------
// Method: constructor
// ----------------------------------------------------------------------------
DebounceButton::DebounceButton(byte pin, int db_msec)
{
   mPin = pin;
   mDebounceDelay = db_msec;
   mLastSts = DBTN_KEYUP;
   mPreviousLev = HIGH;
}

// ----------------------------------------------------------------------------
// Method: begin()
// ----------------------------------------------------------------------------
void DebounceButton::begin(boolean pullup)
{
   pinMode(mPin, (pullup ? INPUT_PULLUP : INPUT));      
}

// ----------------------------------------------------------------------------
// Method: getStatus()
// ----------------------------------------------------------------------------
byte DebounceButton::getStatus()
{
   byte lev = (byte) digitalRead(mPin);

   return ((lev == HIGH) ? DBTN_KEYUP:DBTN_KEYDOWN);
}

// ----------------------------------------------------------------------------
// Method: update()
// ----------------------------------------------------------------------------
byte DebounceButton::update()
{
   byte ret = DBTN_NOCHANGE;

   // Read the digital pin.
   byte lev = (byte) digitalRead(mPin);

   // Set the timestamp when a change occurs.
   if (lev != mPreviousLev)
   {
      mEventTime = millis();
   }
   else if( (unsigned long)(millis() - mEventTime) >= (unsigned long)mDebounceDelay ) 
   {
      // Enters here if the level is stable for a specified time.
      byte sts = ((lev == HIGH) ? DBTN_KEYUP:DBTN_KEYDOWN);

      if (sts != mLastSts)
      {
         ret = sts;
      }

      mLastSts = sts;
   }

   mPreviousLev = lev;

   return ret;
}