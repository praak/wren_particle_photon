// This #include statement was automatically added by the Particle IDE.
#include "Hdc1080.h"

Hdc1080 hdc = Hdc1080();

void setup() {
  Serial.begin(9600);
  Serial.println("Welcome to WREN");

  hdc.begin();
  delay(15);    // let the chip initialize
}


void loop() {
  Serial.print("Temp in F: "); Serial.println(hdc.getTemperatureFahrenheit());
  Serial.print("Temp in C: "); Serial.println(hdc.getTemperatureCelcius());
  Serial.print("Humidity: "); Serial.println(hdc.getHumidity());

  delay(2000);

}
