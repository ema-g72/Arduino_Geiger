/****************************************************************************
* File:     LoopTimer.h
* Project:  Utilities
* Abstract: Defines a class for time expiration check, used to create timed loops.
* 
****************************************************************************/

#ifndef _LOOP_TIMER_H_
#define _LOOP_TIMER_H_

// ----------------------------------------------------------------------------
// class: LoopTimer
//
// This class is used to check when a time interval is elapsed. The expired()
// method shall be called in a loop and returns true once every (ms) milliseconds.
// ----------------------------------------------------------------------------
class LoopTimer
{
public:
   
   // Constructor.
   LoopTimer();

   // Force the expire method to return true at the next call.
   void forceExpired(int ms);

   // Return true if the specified time interval (ms) is elapsed since the last call.
   boolean expired(int ms);

protected:

   unsigned long mPrevMillis;
};

#endif

