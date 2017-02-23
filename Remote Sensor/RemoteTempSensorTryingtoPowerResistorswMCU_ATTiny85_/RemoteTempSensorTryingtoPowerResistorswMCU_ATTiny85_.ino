//Functions with remote sensor to wall unit xbee
//Puts the XBee to sleep using pin 5 as output high (asleep) or input low (awake)
#include <SoftwareSerial.h>

int sensorPos = 1;// (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2) PB2 (Pin 7)
int sensorNeg = 3; // (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 (Pin 2)
int TX = 4; // (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 (Pin 3) 
int RX =  5; // (PCINT5/RESET/ADC0/dW) PB5 (Pin 1)
int power = 1;
int Name = 1;
int XBee_wake = 0;
float voltage, negVoltage, posVoltage, degreesC;
SoftwareSerial XBee(RX,TX);

void setup() {
//   Serial.begin(9600);
  pinMode(power,OUTPUT);
  pinMode(XBee_wake,OUTPUT);
  pinMode(RX,INPUT);
  pinMode(TX,OUTPUT);
  pinMode(sensorPos, INPUT);
  pinMode(sensorNeg, INPUT);
   XBee.begin(9600);
}

void loop() { 
  //  XBee.print("voltage: ");
  //  XBee.print(voltage);
  //  XBee.print("  deg C: ");
  //  XBee.println(degreesC);
  //  XBee.print("  deg F: ");
  
    XBee.print(degreesF(power));
    XBee.print("                ID: ");
    XBee.println(Name);
   
  //  XBee.println("S");
  //  XBee.println(analogRead(sensorPos));
  //  XBee.println(analogRead(sensorNeg));
  //  XBee.println("E"); 
//    delay(200);
    digitalWrite(power, LOW);
    digitalWrite(XBee_wake, HIGH);
  
  for(int i = 0; i < 5; i++)
  {
    delay(1000);
  }
}

float degreesF(int pin)
{
///  delay(2000);
      digitalWrite(power, HIGH);
      digitalWrite(XBee_wake, LOW);
      negVoltage = getVoltage(sensorNeg);
      posVoltage = getVoltage(sensorPos);
      voltage = posVoltage - negVoltage;
      degreesC = -5.3546*voltage*voltage + 31.279*voltage + 21.531;
      return (degreesC * (9.0/5.0) + 32.0);
  }
float getVoltage(int pin)
{
  return (analogRead(pin)* 0.00322265625);//0.0029296875); // Old value = * 0.00322265625);
}
