/****************************************************************************
* File:     FrequencyGen.h
* Project:  Utilities
* Abstract: Generate a square wave on digital pin 3 using Timer2.
* 
****************************************************************************/

#ifndef _FREQUENCY_GEN_H_
#define _FREQUENCY_GEN_H_

#include <Arduino.h>

// ----------------------------------------------------------------------------
// class: FrequencyGen
// This class generates a square wave (50% duty cycle) on output COM2B0 (digital pin 3).
// ----------------------------------------------------------------------------
class FrequencyGen
{
public:

   // Initialize the internal timer.
   void begin();

   // Start the waveform generation.
   void start();
   
   // Stop the waveform generation.
   void stop();   
};

#endif
