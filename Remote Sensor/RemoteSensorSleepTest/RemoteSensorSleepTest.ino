/* Example of sleeping and saving power

  Thanks to: Nick Gammon; 25 May 2011
  Modified on 19 Feb 17 by David Harrison
  For use on Remote Temperature Sensor for Senior Design Project

  This code puts both the ATTiny85, Temperature sensing circuit and Xbee
  to sleep and wakes after X seconds(based on the for loop at the end).
  After waking, the MCU turns on the XBee, waits .5sec, takes temperature readings,
  then sends the data (possibly previous temp readings as well which is
  not implemented atm), then disables the XBee and temp circuit before
  going into sleep mode again.
*/
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>


// Routines to set and clear bits (used in the sleep code(myWatchdogEnable))
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/*********************
  PRESCALER - modifies the clock times. For delay(1000) with Mode 4, delay would be 16x longer.
  Mode 0 - Do nothing - normal 16Mhz clock
  Mode 1 - Clock divide by 2 - 8MHz
  Mode 2 - Clock divide by 4 - 4MHz
  Mode 3 - Clock divide by 8 - 2MHz
  Mode 4 - Clock divide by 16 - 1MHz
  Mode 5 - Clock divide by 32 - 500kHz
  Mode 6 - Clock divide by 64 - 250kHz
  Mode 7 - Clock divide by 128 - 125kHz
  Mode 8 - Clock divide by 256 - 62.5kHz
*********************/
// #define Set_prescaler(x)  (CLKPR = (1<<CLKPCE),CLKPR = x) //for any delays, they are multiplied by the prescaler time
// #define LED 5
// int Prescaler_mode = 4;
// const int A0_pin = A0;


/*********************
  Variable Declaration
*********************/
volatile boolean f_wdt = 1;
int sensorPos = 1;// (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2) PB2 (Pin 7)
int sensorNeg = 3; // (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 (Pin 2)
int TX = 4; // (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 (Pin 3)
int RX =  5; // (PCINT5/RESET/ADC0/dW) PB5 (Pin 1)
int XBee_wake = 0; // (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0) PB0 (Pin 5) *HIGH = asleep LOW = awake
int power = 1;
int Name = 2;
float voltage, negVoltage, posVoltage, degreesC;
String Nam = String(Name);
String degF = String();
String sep = String(':');
SoftwareSerial XBee(RX, TX);


/*********************
  WATCHDOG INTERRUPT
*********************/
ISR(WDT_vect)
{
  f_wdt = 1;  // set global flag
}


/*********************
  myWatchdogEnable
*********************/
void myWatchdogEnable(const byte interval)
{
  /*** Setup the WDT ***/

  /* Clear the reset flag. */
  MCUSR &= ~(1 << WDRF);

  /* In order to change WDE or the prescaler, we need to
     set WDCE (This will allow updates for 4 clock cycles).
  */
  WDTCR |= (1 << WDCE) | (1 << WDE);

  /* set new watchdog timeout prescaler value */
  WDTCR = 1 << WDP0 | 1 << WDP3; /* 8.0 seconds */

  /* Enable the WD interrupt (note no reset). */
  WDTCR |= _BV(WDIE);
  //  MCUSR = 0;                          // reset various flags
  //  WDTCSR |= 0b00011000;               // see docs, set WDCE, WDE
  //  WDTCSR =  0b01000000 | interval;    // set WDIE, and appropriate delay
  //
  //  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  wdt_reset();

  set_sleep_mode (SLEEP_MODE_PWR_DOWN);

  sleep_enable();

  sleep_mode();            // now goes to Sleep and waits for the interrupt

  sleep_disable();                     // System continues execution here when watchdog timed out

  //  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON

}

/*********************
  float getVoltage
*********************/
float getVoltage(int pin)
{
  return (analogRead(pin) * 0.00322265625); //0.0029296875); // Old value = * 0.00322265625);
}

/*********************
  float degreesF
*********************/
int degreesF(int pin)
{
  digitalWrite(XBee_wake, LOW);
  delay(500);
  digitalWrite(power, HIGH);
  negVoltage = getVoltage(sensorNeg);
  posVoltage = getVoltage(sensorPos);
  voltage = posVoltage - negVoltage;
  degreesC = -5.3546 * voltage * voltage + 31.279 * voltage + 22.231; //21.531;
  return (degreesC * (9.0 / 5.0) + 32.0);
}


/*********************
  VOID SETUP
*********************/
void setup()
{
  //  Set_prescaler(Prescaler_mode);  // 1/16 clock speed
  //  int i;
  //  for(i = 0; i <= 18; i++)  // all pins set as input with pullup resistor, but causes temp values to increase by 20F or so
  //  {
  //    pinMode(i, INPUT);
  //    digitalWrite(i, HIGH);
  //  }
  //Set Pins that will be used
  pinMode(power, OUTPUT);
  pinMode(XBee_wake, OUTPUT);
  pinMode(RX, INPUT);
  pinMode(TX, OUTPUT);
  pinMode(sensorPos, INPUT);
  pinMode(sensorNeg, INPUT);
  digitalWrite(XBee_wake, LOW);
  XBee.begin(9600);
}


/*********************
  VOID LOOP
*********************/
void loop()
{
  if (f_wdt == 1)
  { // wait for timed out watchdog / flag is set when a watchdog timeout occurs

    f_wdt = 0;       // reset flag

    //Do stuff here
    //    XBee.print("                ID: ");
    degF = String(degreesF(power));
    XBee.println(Nam + sep + degF);
    //    XBee.print(":");
    //    XBee.println();
    delay(500);
    digitalWrite(power, LOW);
    digitalWrite(XBee_wake, HIGH);
  }
  /*********************
     SLEEP BIT PATTERNS
     1 second:  0b000110
     2 seconds: 0b000111
     4 seconds: 0b100000
     8 seconds: 0b100001
   *********************/
  // Set the ports to be inputs - saves more power
  //  pinMode(power,INPUT);
  //  pinMode(TX,INPUT);

  cbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter OFF only 1x per sleep
  // sleep for a total of 32 seconds (8*4)
  int i;
  for (i = 0; i < 1; i++) // Multiplier for sleep timer
  {
    myWatchdogEnable (0b100001);  // 8 seconds
  }
  sbi(ADCSRA, ADEN);                   // switch Analog to Digitalconverter ON
  // Set the ports to be output again
  //  pinMode(power,OUTPUT);
  //  pinMode(TX,OUTPUT);
} // end of loop
