/****************************************************************************
* File:     ArduinoGeiger.ino
* Project:  ArduinoGeiger
* Abstract: Main sketch of the ArduinoGeiger firmware.
*
****************************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DebounceButton.h"
#include "EventAction.h"
#include "LoopTimer.h"
#include "FrequencyGen.h"
#include "Geiger.h"

// Firmware version
const char FW_VERSION[] = "1.0";

// Digital pins assignment
const byte PIN_EVT_LED  = 4;
const byte PIN_BTN_OPT  = 6;
const byte PIN_BTN_SEL  = 7;
const byte INT_NUMBER   = 0; // INT0 = digital pin 2.

// LED flash duration (msec).
const byte LED_FLASH_MSEC = 10;
// Buzzer sound duration (msec).
const byte BUZZER_TIC_MSEC = 10;
// I2C address of LCD.
const byte LCD_I2C_ADDR = 0x38;
// Number of LCD columns.
const byte LCD_NUM_COLUMNS = 16;
// Buttons debounce time (msec)
const byte BTN_DEBOUNCE_MSEC = 16;
// Event loop period (msec)
const byte READ_EVT_MSEC = 10;
// Initial LCD screen duration (msec)
const uint16_t INIT_SCREEN_MSEC = 1500;
// LCD screen update rate (msec)
const uint16_t UPDATE_SCREEN_MSEC = 1000;

// State machine constants
const byte STATE_RATE      = 1;
const byte STATE_TOTAL     = 2;
const byte STATE_TOTAL_1M  = 3;
const byte STATE_TOTAL_2M  = 4;
const byte STATE_SET_AUDIO = 5;
const byte STATE_SET_LIGHT = 6;

// Buttons events (FUNCTION and SELECTION)
const byte BTN_OPT  = 1;
const byte BTN_SEL  = 2;
const byte BTN_NONE = 3;

// Local function prototypes
void displayInitScreen();
void readGeigerEvents();
void isr_geiger();
byte readButtons();
void initMeasureState();
void initSetupState(const __FlashStringHelper *txt);
void executeStateRate(byte btn);
void executeStateTotal(byte btn, uint16_t seconds, byte symbol, byte next);
void executeStateSetLight(byte btn);
void executeStateSetAudio(byte btn);
void eventLedOn();
void eventLedOff();
void eventBuzzerOn();
void eventBuzzerOff();
byte getNumberOfDigits(uint32_t num);
byte printLCDLeadingZeroNumber(uint32_t num, byte width);
void printLCD(byte col, byte row, const __FlashStringHelper *txt);
void printLCDBlanks(byte num);
void lcdAudioOnOff(boolean on);
void lcdShowRateLabels();
void lcdShowSelectionOnOff(boolean on);
void lcdUpdateDoseRate(uint32_t cpm, uint32_t nsv_h);
void lcdUpdateTimer(byte symbol, uint32_t sec, uint32_t start);

// Global variables
byte     gCurrentState;
boolean  gEnterNewState;
boolean  gBackLightOn;
boolean  gAudioOn;

// Global objects
DebounceButton    gBtnOpt(PIN_BTN_OPT, BTN_DEBOUNCE_MSEC);
DebounceButton    gBtnSel(PIN_BTN_SEL, BTN_DEBOUNCE_MSEC);
LiquidCrystal_I2C gAppLCD(LCD_I2C_ADDR, LCD_NUM_COLUMNS, 2);
EventAction       gLedAction(eventLedOn, eventLedOff);
EventAction       gBuzzerAction(eventBuzzerOn, eventBuzzerOff);
LoopTimer         gReadEventTimer;
LoopTimer         gAuxTimer;
FrequencyGen      gBuzzer;
Geiger            gGeigerObj;

// Interface between ISR and main program.
volatile uint16_t gGeigerCounter;

// LCD custom characters.
const byte SYM_AUDIO = 0;
byte gSymAudio[8] = { B00000, B00010, B01110, B11110, B11110, B01110, B00010, B00000 };
const byte SYM_TIMER_0 = 1;
byte gSymTimer0[8] = { B11111, B10001, B01010, B00100, B01110, B11011, B11111, B00000 };
const byte SYM_TIMER_1 = 2;
byte gSymTimer1[8] = { B00101, B01101, B00100, B00100, B01110, B00000, B11111, B00000 };
const byte SYM_TIMER_2 = 3;
byte gSymTimer2[8] = { B11101, B00101, B11100, B10000, B11100, B00000, B11111, B00000 };

// ----------------------------------------------------------------------------
// Function: setup()
// Arduino "setup" entry point.
// ----------------------------------------------------------------------------
void setup()
{
   // Initialize LCD.
   gAppLCD.init();
   // Create custom characters.
   gAppLCD.createChar(SYM_AUDIO, gSymAudio);
   gAppLCD.createChar(SYM_TIMER_0, gSymTimer0);
   gAppLCD.createChar(SYM_TIMER_1, gSymTimer1);
   gAppLCD.createChar(SYM_TIMER_2, gSymTimer2);

   // Initialize buttons.
   gBtnOpt.begin(true);
   gBtnSel.begin(true);
   
   // Initialize buzzer.
   gBuzzer.begin();

   // Set digital output pins.
   pinMode(PIN_EVT_LED, OUTPUT);
   
   // Attach interrupt service routine.
   attachInterrupt(INT_NUMBER, isr_geiger, FALLING);

   // Set initial state.
   gCurrentState = STATE_RATE;
   gGeigerCounter = 0;
   gEnterNewState = true;
   gBackLightOn = false;   
   gAudioOn = true;

   // Show startup screen.
   displayInitScreen();   
}

// ----------------------------------------------------------------------------
// Function: loop()
// Arduino "loop" entry point.
// ----------------------------------------------------------------------------
void loop()
{
   // Read buttons.
   byte btn = readButtons();

   // Execute state machine.
   switch (gCurrentState)
   { 
      case STATE_RATE:
         executeStateRate(btn);
         break;
      case STATE_TOTAL:
         executeStateTotal(btn, 0, SYM_TIMER_0, STATE_TOTAL_1M);
         break;
      case STATE_TOTAL_1M:
         executeStateTotal(btn, 60, SYM_TIMER_1, STATE_TOTAL_2M);
         break;
      case STATE_TOTAL_2M:
         executeStateTotal(btn, 120, SYM_TIMER_2, STATE_SET_LIGHT);
         break;
      case STATE_SET_LIGHT:
         executeStateSetLight(btn);
         break;
      case STATE_SET_AUDIO:
         executeStateSetAudio(btn);
         break;
      default:
         break;         
   }

   // Read event counter from geiger tube.
   readGeigerEvents();

   // Update actions.
   gLedAction.update();
   gBuzzerAction.update();   
}

// ----------------------------------------------------------------------------
// Function: isr_geiger()
// Interrupt Service Routine for geiger tube events.
// ----------------------------------------------------------------------------
void isr_geiger()
{
   gGeigerCounter ++;
}

// ----------------------------------------------------------------------------
// Function: executeStateRate(byte btn)
// This function is executed when the system is in "Rate" state. The screen is
// updated with the CPM (Count per Minutes) and dose rate (microSievert per hour).
// These values are computed every 3 seconds as a moving average of the last
// 30 seconds. Press the SEL button to reset the average.
// ----------------------------------------------------------------------------
void executeStateRate(byte btn)
{
   if( gEnterNewState )
   {
      initMeasureState();
   }

   if( btn == BTN_OPT )
   {
      gEnterNewState = true;
      gCurrentState = STATE_TOTAL;
   }
   // SEL button, reset the moving average. 
   else if( btn == BTN_SEL )
   {
      gGeigerObj.resetCount();
      gAuxTimer.forceExpired(UPDATE_SCREEN_MSEC);
   }
   else if( gAuxTimer.expired(UPDATE_SCREEN_MSEC) )
   {
      // Get radiation rate.
      uint32_t cpm, nsv_h;
      gGeigerObj.getRateAverage( &cpm, &nsv_h);

      // Update CPM and dose rate.
      lcdUpdateDoseRate(cpm, nsv_h);
   }
}

// ----------------------------------------------------------------------------
// Function: executeStateTotal(byte btn, uint16_t seconds, byte symbol, byte next)
// This function is executed when the system is in "Total" state. The screen is
// updated with the number of events and the dose rate.
// The display shows the 'symbol' character before the time indicator, the 'next'
// parameters is the state reached at the OPT command.
// If the parameter 'seconds' is zero the timer starts from 0 to infinite, if 
// greater than zero it starts from 'seconds' decreasing up to 0.
// Press the SEL button to stop the timer, at the second pressure, the counter is
// restarted.
// ----------------------------------------------------------------------------
void executeStateTotal(byte btn, uint16_t seconds, byte symbol, byte next)
{
   // Local static variables.
   static uint32_t lStartTimerMs;
   static boolean lTotalTimerStop;

   // Executed once when entering this state.
   if( gEnterNewState )
   {
      initMeasureState();

      lStartTimerMs = millis();
      lTotalTimerStop = false;
   }

   // OPT button, goto next state.
   if( btn == BTN_OPT )
   {
      gEnterNewState = true;
      gCurrentState = next;
   }
   // SEL button, stop/restart timer. 
   else if( btn == BTN_SEL )
   {
      if( lTotalTimerStop )
      {
         lStartTimerMs = millis();
         gGeigerObj.resetCount();
         lTotalTimerStop = false;
      }
      else
      {
         gAppLCD.setCursor(10,0);
         gAppLCD.print("*");

         lTotalTimerStop = true;
      }
   }
   // Refresh LCD screen.
   else if( !lTotalTimerStop && gAuxTimer.expired(UPDATE_SCREEN_MSEC) )
   {
      // Elapsed time in milliseconds.
      uint32_t sec = (millis() - lStartTimerMs)/1000;

      // Infinite time interval.
      if( seconds == 0 )
      {
         lcdUpdateTimer(symbol, sec, 0);
      }
      else // Stop counter at the end of time interval .
      {
         lcdUpdateTimer(symbol, sec, seconds);         

         if( sec >= seconds )
         {
            gAppLCD.setCursor(10,0);
            gAppLCD.print("*");

            lTotalTimerStop = true;         
         }
      }

      // Get radiation rate.
      uint32_t cpm, nsv_h;
      gGeigerObj.getRateTimer( &cpm, &nsv_h);

      // Update CPM and dose rate.
      lcdUpdateDoseRate(cpm, nsv_h);
   }
}

// ----------------------------------------------------------------------------
// Function: executeStateSetLight(byte btn)
// This function is executed when the system is in "Set Light" state. 
// Press the SEL button to switch ON/OFF the LCD backlight.
// ----------------------------------------------------------------------------
void executeStateSetLight(byte btn)
{
   if( gEnterNewState )
   {
      initSetupState(F("BACKLIGHT "));

      lcdShowSelectionOnOff(gBackLightOn);
   }

   if( btn == BTN_OPT )
   {
      gEnterNewState = true;
      gCurrentState = STATE_SET_AUDIO;
   }
   else if( btn == BTN_SEL )
   {
      gBackLightOn = !gBackLightOn;

      lcdShowSelectionOnOff(gBackLightOn);

      if( gBackLightOn )
      {
         gAppLCD.backlight();
      }
      else
      {
         gAppLCD.noBacklight();
      }
   }
}

// ----------------------------------------------------------------------------
// Function: executeStateSetAudio(byte btn)
// This function is executed when the system is in "Set Audio" state. 
// Press the SEL button to switch ON/OFF the buzzer.
// ----------------------------------------------------------------------------
void executeStateSetAudio(byte btn)
{
   if( gEnterNewState )
   {
      initSetupState(F("AUDIO "));

      lcdShowSelectionOnOff(gAudioOn);
   }

   if( btn == BTN_OPT )
   {
      gEnterNewState = true;
      gCurrentState = STATE_RATE;
   }
   else if( btn == BTN_SEL )
   {
      lcdAudioOnOff(!gAudioOn);

      lcdShowSelectionOnOff(gAudioOn);
   }
}

// ----------------------------------------------------------------------------
// Function: readGeigerEvents()
// This function reads the number of events registered by the interrupt routine.
// It is executed at the specified rate (READ_EVT_MSEC) and triggers the actions
// associated with an event (LED flash and sound tic)
// ----------------------------------------------------------------------------
void readGeigerEvents()
{
   if( gReadEventTimer.expired(READ_EVT_MSEC) )
   {
      // Reset the isr counter.
      uint16_t num = gGeigerCounter;
      gGeigerCounter = 0;

      // Add to Geiger object.
      gGeigerObj.addCount(num);

      // Perform actions on event.
      if( num > 0 )
      {
         gLedAction.start(LED_FLASH_MSEC);

         if( gAudioOn )
         {
            gBuzzerAction.start(BUZZER_TIC_MSEC);
         }
      }
   }
}

// ----------------------------------------------------------------------------
// Function: initMeasureState()
// Initialize a radiation measure. 
// ----------------------------------------------------------------------------
void initMeasureState()
{
   gAppLCD.clear();
   lcdAudioOnOff(gAudioOn);
   lcdShowRateLabels();

   gAuxTimer.forceExpired(UPDATE_SCREEN_MSEC);
   gGeigerObj.resetCount();

   gEnterNewState = false;
}

// ----------------------------------------------------------------------------
// Function: initSetupState()
// Initialize a setup state. 
// ----------------------------------------------------------------------------
void initSetupState(const __FlashStringHelper *txt)
{
   gAppLCD.clear();
   lcdAudioOnOff(gAudioOn);
   printLCD(0,0, txt);

   gEnterNewState = false;
}

// ----------------------------------------------------------------------------
// Function: readButtons()
// Read the two buttons and returns the selected one.
// ----------------------------------------------------------------------------
byte readButtons()
{
   byte opt = gBtnOpt.update();
   byte sel = gBtnSel.update();

   // OPTION button.
   if( opt == DBTN_KEYDOWN )
   {
      return BTN_OPT;
   }

   // SELECTION button.
   if( sel == DBTN_KEYDOWN )
   {
      return BTN_SEL;
   }

   return BTN_NONE;
}

// ----------------------------------------------------------------------------
// Function: lcdAudioOnOff(boolean on)
// Set the Audio on/off variable and display the status on LCD.
// ----------------------------------------------------------------------------
void lcdAudioOnOff(boolean on)
{
   gAudioOn = on;
   gAppLCD.setCursor(15,1);
   
   if(gAudioOn)
   {
      gAppLCD.write(SYM_AUDIO);
   }
   else
   {
      gAppLCD.print(F(" "));   
   }
}

// ----------------------------------------------------------------------------
// Function: lcdShowRateLabels()
// Show the count rate and dose rate labels on LCD.
// ----------------------------------------------------------------------------
void lcdShowRateLabels()
{
   printLCD(0,0, F("CPM "));
   printLCD(0,1, F("uSv/h "));
}

// ----------------------------------------------------------------------------
// Function: lcdUpdateDoseRate(uint32_t cpm, uint32_t nsv_h)
// Update the dose rate fields with "cpm" and uSv/h (nsv_h = nanoSievert per hour).
// ----------------------------------------------------------------------------
void lcdUpdateDoseRate(uint32_t cpm, uint32_t nsv_h)
{
   uint32_t tmp;
   byte ndig; 

   // Print Counts per Minute.
   gAppLCD.setCursor(4,0);
   gAppLCD.print(cpm);

   // Clear cpm field.
   printLCDBlanks( 6 - getNumberOfDigits(cpm));

   // Print micro Sievert.
   gAppLCD.setCursor(6,1);
   
   // Integer part.
   tmp = nsv_h/1000;
   
   ndig = getNumberOfDigits(tmp);

   gAppLCD.print(tmp);
   gAppLCD.print(F("."));

   // Fractional part.
   tmp = nsv_h%1000;  

   printLCDLeadingZeroNumber(tmp,3);
   ndig += 4;

   // Clear uSv/h field.
   printLCDBlanks(8 - ndig);
}

// ----------------------------------------------------------------------------
// Function: lcdUpdateTimer(byte symbol, uint32_t sec, uint32_t start)
// Update the timer field with the time in format MM:SS.
// ----------------------------------------------------------------------------
void lcdUpdateTimer(byte symbol, uint32_t sec, uint32_t start)
{
   // If start > 0 decrease the timer.
   if( start > 0 )
   {
      sec = start - sec;
   }

   // Update elapsed time
   byte min = (byte) (sec/60);
   sec %= 60;

   gAppLCD.setCursor(10,0);
   gAppLCD.write(symbol);

   printLCDLeadingZeroNumber(min, 2);
   gAppLCD.print(F(":"));
   printLCDLeadingZeroNumber((byte) sec, 2);
}

// ----------------------------------------------------------------------------
// Function: lcdShowSelectionOnOff(boolean on)
// Show the ON/OFF selection on LCD display.
// ----------------------------------------------------------------------------
void lcdShowSelectionOnOff(boolean on)
{
   gAppLCD.setCursor(0,1);
   gAppLCD.print(F("> "));
   gAppLCD.print( on ? F("ON ") : F("OFF") );
}

// ----------------------------------------------------------------------------
// Function: displayInitScreen()
// Show the startup screen on LCD.
// ----------------------------------------------------------------------------
void displayInitScreen()
{
   printLCD(0,0,F("ArduinoGeiger")); 
   printLCD(0,1,F("Ver. ")); 
   gAppLCD.print(FW_VERSION);

   delay(INIT_SCREEN_MSEC);
}

// ----------------------------------------------------------------------------
// Function: getNumberOfDigits(uint32_t num)
// Return the number of digits of the number (num).
// ----------------------------------------------------------------------------
byte getNumberOfDigits(uint32_t num)
{
   byte ndigits = 0;

   if( num == 0 )
   {
      ndigits ++;
   }
   else
   {
      for( ; num != 0; num /= 10, ndigits++ );
   }

   return ndigits;
}

// ----------------------------------------------------------------------------
// Function:  printLCDBlanks(byte num)
// Print a number (num) of space characters on LCD.
// ----------------------------------------------------------------------------
void printLCDBlanks(byte num)
{
   char tmp[LCD_NUM_COLUMNS+1];
   
   num = min(num, LCD_NUM_COLUMNS);
   
   tmp[num] = '\0';

   for(byte i=0; i<num; ++i)
   {
      tmp[i] = ' ';
   }

   gAppLCD.print(tmp);
}

// ----------------------------------------------------------------------------
// Function: printLCDLeadingZeroNumber(uint32_t num, byte width)
// Print on LCD a number (num) with leading zeroes up to (width) digits.
// Return the number of characters printed on LCD.
// ----------------------------------------------------------------------------
byte printLCDLeadingZeroNumber(uint32_t num, byte width)
{
   // Count number of digits.
   byte ndigits = getNumberOfDigits(num);

   // Number of leading zero.
   int8_t lzero = (int8_t)(width - ndigits);

   for(; lzero >0 ; lzero--)
   {
      gAppLCD.print("0");   
   }

   gAppLCD.print(num, DEC);

   return max(ndigits,width);
}

// ----------------------------------------------------------------------------
// Function: void printLCD(byte col, byte row, const __FlashStringHelper *txt)
// Print a text on LCD at the cursor position specified by column (col) 
// and row (row) numbers.
// ----------------------------------------------------------------------------
void printLCD(byte col, byte row, const __FlashStringHelper *txt)
{
   gAppLCD.setCursor(col, row);
   gAppLCD.print(txt);
}

// ----------------------------------------------------------------------------
// Function: eventLedOn()
// Callback executed when the LED Action starts.
// ----------------------------------------------------------------------------
void eventLedOn()
{
   digitalWrite(PIN_EVT_LED, HIGH);
}

// ----------------------------------------------------------------------------
// Function: eventLedOff()
// Callback executed when the LED Action ends.
// ----------------------------------------------------------------------------
void eventLedOff()
{
   digitalWrite(PIN_EVT_LED, LOW);
}

// ----------------------------------------------------------------------------
// Function: eventBuzzerOn()
// Callback executed when the Buzzer Action starts.
// ----------------------------------------------------------------------------
void eventBuzzerOn()
{
   gBuzzer.start();
}

// ----------------------------------------------------------------------------
// Function: eventBuzzerOff()
// Callback executed when the Buzzer Action ends.
// ----------------------------------------------------------------------------
void eventBuzzerOff()
{
   gBuzzer.stop();
}
