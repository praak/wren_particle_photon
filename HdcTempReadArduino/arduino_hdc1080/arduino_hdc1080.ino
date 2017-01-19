#include <Wire.h>
#include "HDC1080.h"
#include <SoftwareSerial.h>

SoftwareSerial xbee(2,3); // RX,TX

HDC1080 hdc;

void setup()
{
	Serial.begin(9600);
	Serial.println("HDC1080 Arduino Test");
	hdc.begin(0x40);
        xbee.begin(9600);
}

void loop()
{
	Serial.print("T=");
	Serial.print(hdc.readTemperature());
	Serial.print("C, RH=");
	Serial.print(hdc.readHumidity());
	Serial.println("%");
        float c = hdc.readTemperature();
//        xbee.println('Temp:');        
        xbee.println(c);

        delay(1000);
}
