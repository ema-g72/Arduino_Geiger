/****************************************************************************
* File:     DebounceButton.h
* Project:  Utilities
* Abstract: Implements a software debouncer for mechanical buttons.
* 
****************************************************************************/

#ifndef _DEBOUNCE_BUTTON_H_
#define _DEBOUNCE_BUTTON_H_

#include <Arduino.h>

// Push button status.
#define DBTN_KEYUP    0
#define DBTN_KEYDOWN  1
#define DBTN_NOCHANGE 2

// ----------------------------------------------------------------------------
// class: DebounceButton
//
// Software debouncer for push buttons. It filters electrical spikes due to 
// mechanical action by waiting for a stable digital level for a specified time.
// The update() function shall be called in a loop to continously monitor the
// digital pin. It returns the button event: DBTN_KEYUP, DBTN_KEYDOWN, DBTN_NOCHANGE
// ----------------------------------------------------------------------------
class DebounceButton
{
public:
    
   // Constructor, specify the digital pin number (pin) and debounce time (ms)
   // in milliseconds.
   DebounceButton(byte pin, int db_msec);

   // Initialize the object, specify if the internal pullup resistors shall be enabled.
   void begin(boolean pullup);

   // Return the button status without debouce filter.
   byte getStatus();

   // Update the digital pin reading and returns the current status.
   byte update();

protected:

   byte mPin;
   byte mPreviousLev;
   byte mLastSts;
   unsigned long mEventTime;
   int mDebounceDelay;
};

#endif
