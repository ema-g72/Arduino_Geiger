/****************************************************************************
* File:     FrequencyGen.cpp
* Project:  Utilities
* Abstract: Generate a square wave on digital pin 3 using Timer2.
* 
****************************************************************************/

#include "FrequencyGen.h"

const byte PIN_WAVEFORM_B = 3;

// ----------------------------------------------------------------------------
// Method: begin()
// ----------------------------------------------------------------------------
void FrequencyGen::begin()
{
   // Timer2 output pin 3.
   pinMode(PIN_WAVEFORM_B, OUTPUT);

   TCCR2A = 0;  
   TCCR2B = 0;  
   
   // WGM21 = CTC mode.
   TCCR2A = _BV(WGM21);
   // CS2[0:2] = prescaler (001=F/1, 010=F/8, 011=F/32, 100=F/64, 101=F/128, 110=F/256, 111=F/1024).
   TCCR2B = _BV(CS21) | _BV(CS20);
   // Counter TOP limit. Frequency: F_CPU/((OCRA+1)*2*prescaler), OCR2A=61 -> 4032 Hz
   OCR2A = 61; 
}

// ----------------------------------------------------------------------------
// Method: start()
// ----------------------------------------------------------------------------
void FrequencyGen::start()
{
   // COM2B0 = Toggle OC2B on Compare Match.
   TCCR2A |= _BV(COM2B0);
}

// ----------------------------------------------------------------------------
// Method: stop()
// ----------------------------------------------------------------------------
void FrequencyGen::stop()
{
   TCCR2A &= ~(_BV(COM2B0));  
   digitalWrite(PIN_WAVEFORM_B, LOW);
}
