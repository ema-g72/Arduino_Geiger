/****************************************************************************
* File:     EventAction.cpp
* Project:  Utilities
* Abstract: .
* 
****************************************************************************/

#include "EventAction.h"

// ----------------------------------------------------------------------------
// Method: constructor
// ----------------------------------------------------------------------------
EventAction::EventAction(ActionCallback start_cb, ActionCallback stop_cb)
{
   mStart = false;
   mStartFunction = start_cb;
   mStopFunction = stop_cb;
}

// ----------------------------------------------------------------------------
// Method: start(uint16_t msec)
// ----------------------------------------------------------------------------
void EventAction::start(uint16_t msec)
{
   mDurationMsec = msec;
   mStartTimeMsec = millis();
   mStart = true;

   mStartFunction();
}

// ----------------------------------------------------------------------------
// Method: update()
// ----------------------------------------------------------------------------
void EventAction::update()
{
   unsigned long now = millis();

   if( mStart && ((now - mStartTimeMsec) >= (unsigned long) mDurationMsec ))
   {
      mStart = false;

      mStopFunction();
   }
}

