/****************************************************************************
* File:     EventAction.h
* Project:  Utilities
* Abstract: .
* 
****************************************************************************/

#ifndef _EVENT_ACTION_H_
#define _EVENT_ACTION_H_

#include <Arduino.h>

// ----------------------------------------------------------------------------
// class: EventAction
//
// 
// ----------------------------------------------------------------------------
class EventAction
{
public:
   typedef void (*ActionCallback)(void);

   // Constructor, specify the callback to be executed at action start and stop.
   EventAction( ActionCallback start_cb, ActionCallback stop_cb);
   
   // Switch on the digital pin for the duration in milliseconds (msec).
   void start(uint16_t msec);
   
   // Update the action execution.
   void update();

protected:

   boolean mStart;
   uint16_t mDurationMsec;
   unsigned long mStartTimeMsec;
   ActionCallback mStartFunction;
   ActionCallback mStopFunction;
};

#endif
