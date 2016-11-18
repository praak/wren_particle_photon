// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_HDC1000.h"

Adafruit_HDC1000 hdc = Adafruit_HDC1000();

float tempconvert(float);

void setup() {
  // prints to USB port on Photon   RMB
  Serial.begin(9600);
  /*Serial.println("HDC10xx test");*/
  Serial.println("Welcome to WREN");

  hdc.begin();
  delay(15);    // let the chip initialize

// Not using   couldn't find sensor
  /*if (!hdc.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }*/
}


void loop() {
  Serial.print("Temp in C: "); Serial.print(hdc.readTemperature());
  //Serial.print("\t\tHum: "); Serial.println(hdc.readHumidity());

  // here is a different approach   RMB
  hdc.ReadTempHumidity();     // one conversion, one read version

  float temp = hdc.GetTemperature();
  Serial.print("\nTemp in F: "); Serial.print(tempconvert(temp));
  Serial.print("\nHumidity: "); Serial.print(hdc.GetHumidity());
  /*Serial.print("\tBatteryLOW: ");
  if (hdc.batteryLOW()) Serial.println("TRUE");
  else Serial.println("FALSE");*/
  delay(500);

}

float tempconvert(float celcius) {
  float fahrenheit = (celcius * 9/5) + 32;
  return fahrenheit;
}
