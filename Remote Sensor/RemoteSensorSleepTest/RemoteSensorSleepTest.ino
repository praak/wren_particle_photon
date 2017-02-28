/* Example of sleeping and saving power
 
 Thanks to: Nick Gammon; 25 May 2011
 Modified on 19 Feb 17 by David Harrison
 For use on Remote Temperature Sensor for Senior Design Project

 This code puts both the ATTiny85, Temperature sensing circuit and Xbee
 to sleep and wakes after X seconds(based on the for loop at the end).
 After waking, the MCU takes temperature readings, turns on the XBee,
 then sends the data (possibly previous temp readings as well which is
 not implemented atm), then disables the XBee and temp circuit before 
 going into sleep mode again.
*/
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

// Routines to set and clear bits (used in the sleep code(myWatchdogEnable))
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/*********************
* PRESCALER - modifies the clock times. For delay(1000) with Mode 4, delay would be 16x longer.
* Mode 0 - Do nothing - normal 16Mhz clock
* Mode 1 - Clock divide by 2 - 8MHz
* Mode 2 - Clock divide by 4 - 4MHz
* Mode 3 - Clock divide by 8 - 2MHz
* Mode 4 - Clock divide by 16 - 1MHz
* Mode 5 - Clock divide by 32 - 500kHz
* Mode 6 - Clock divide by 64 - 250kHz
* Mode 7 - Clock divide by 128 - 125kHz
* Mode 8 - Clock divide by 256 - 62.5kHz
*********************/
// #define Set_prescaler(x)  (CLKPR = (1<<CLKPCE),CLKPR = x) //for any delays, they are multiplied by the prescaler time
// #define LED 5
// int Prescaler_mode = 4;
// const int A0_pin = A0;
 volatile boolean f_wdt = 1;


/*********************
* WATCHDOG INTERRUPT
*********************/
ISR(WDT_vect) 
{
  f_wdt = 1;  // set global flag
}


/*********************
* myWatchdogEnable
*********************/
void myWatchdogEnable(const byte interval) 
{ 
  /*** Setup the WDT ***/
  
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCR |= _BV(WDIE); 
//  MCUSR = 0;                          // reset various flags
//  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
//  WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay
//  
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  wdt_reset();
  
  set_sleep_mode (SLEEP_MODE_PWR_DOWN); 
   
  sleep_enable();
  
  sleep_mode();            // now goes to Sleep and waits for the interrupt

  sleep_disable();                     // System continues execution here when watchdog timed out 
  
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
  
} 


/*********************
* VOID SETUP
*********************/
void setup()
{
//  Set_prescaler(Prescaler_mode);  // 1/16 clock speed
  int i;
  for(i = 0; i <= 5; i++)  // all pins set as input with pullup resistor
  {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  } 
  //Set Pins that will be used
//  pinMode (LED, OUTPUT);
//  pinMode(A0_pin, INPUT);
}


/*********************
* VOID LOOP
*********************/
void loop()
{
   if (f_wdt == 1) 
   {  // wait for timed out watchdog / flag is set when a watchdog timeout occurs
    
    f_wdt = 0;       // reset flag

    //Do stuff here
   }
  /*********************
   * SLEEP BIT PATTERNS
   * 1 second:  0b000110
   * 2 seconds: 0b000111
   * 4 seconds: 0b100000
   * 8 seconds: 0b100001
   *********************/
    // Set the ports to be inputs - saves more power
//    pinMode(txPin, INPUT);  
//    pinMode(redled, INPUT);
//    pinMode(buzzLedSw, INPUT);        // First want to read the switch
//    pinMode(FETdriver, INPUT);
    
  // sleep for a total of 32 seconds (8*4)
  int i;
  for (i = 0; i < 4; i++)
  {  
    myWatchdogEnable (0b100001);  // 8 seconds
  }

    // Set the ports to be output again
//    pinMode(rxPin, INPUT);
//    pinMode(txPin, OUTPUT);  
//    pinMode(redled, OUTPUT);
//    pinMode(buzzLedSw, OUTPUT);        // First want to read the switch
//    pinMode(FETdriver, OUTPUT);
} // end of loop
