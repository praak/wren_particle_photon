/***************************************************
  This is a library for the HDC1080 Humidity & Temp Sensor

  Designed specifically to work with the HDC1008 sensor from Adafruit
  ----> https://www.adafruit.com/products/2635

  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  Modified for Photon  needs application.h for types  RMB
 ****************************************************/
#include "application.h"
#include "Hdc1080.h"


Hdc1080::Hdc1080() {
}


boolean Hdc1080::begin(uint8_t addr) {
  _i2caddr = addr;

  Wire.begin();

  reset();
  if (read16(HDC1080_MANUFID) != 0x5449) return false;
  if (read16(HDC1080_DEVICEID) != 0x1000) return false;
  return true;
}



void Hdc1080::reset(void) {
  // reset,combined temp/humidity measurement,and select 14 bit temp & humidity resolution
  // heater not needed for accurate humidity readings     RMB
  uint16_t config = HDC1080_CONFIG_RST | HDC1080_CONFIG_MODE | HDC1080_CONFIG_TRES_14 | HDC1080_CONFIG_HRES_14 ;

  Wire.beginTransmission(_i2caddr);
  Wire.write(HDC1080_CONFIG);   // set pointer register to configuration register   RMB
  Wire.write(config>>8);        // now write out 2 bytes MSB first    RMB
  Wire.write(config&0xFF);
  Wire.endTransmission();
  delay(15);
}

float Hdc1080::getHumidity(void) {
  // reads both temp and humidity but masks out temp in highest 16 bits
  // does not set private variable
  float hum = (read32(HDC1080_TEMP, 20) & 0xFFFF);

  hum /= 65536;
  hum *= 100;

  return hum;
}

float Hdc1080::getTemperatureCelcius(void) {
  // getter function to access private temp variable
  uint32_t rt = read32(HDC1080_TEMP, 20) >> 16; // relative temperature
  float (temp = (rt));        // convert to temp first
  temp /= 65536;
  temp *= 165;
  temp -= 40;
  return temp ;
}

float Hdc1080::getTemperatureFahrenheit(void) {
  // getter function to access private temp variable
  float celcius = getTemperatureCelcius();
  float fahrenheit = (celcius * 9/5) + 32;
  return fahrenheit;
}

// Add ability to test battery voltage, useful in remote monitoring, TRUE if <2.8V
// usually called after Temp/Humid reading  RMB
// Thanks to KFricke for micropython-hdc1008 example on GitHub
boolean Hdc1080::batteryLOW(void)  {
  // set private variable, don't need delay to read Config register
  battLOW = (read16(HDC1080_CONFIG, 0));

  battLOW &= HDC1080_CONFIG_BATT;   // mask off other bits, bit 11 will be 1 if voltage < 2.8V

  if (battLOW > 0) return true;
  return  false;
}

/*********************************************************************/

uint16_t Hdc1080::read16(uint8_t a, uint8_t d) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(a);
  Wire.endTransmission();
  delay(d);
  Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)2);
  uint16_t r = Wire.read();
  r <<= 8;
  r |= Wire.read();
  //Serial.println(r, HEX);
  return r;
}

uint32_t Hdc1080::read32(uint8_t a, uint8_t d) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(a);
  Wire.endTransmission();
  // delay was hardcoded as 50, should use d   RMB
  delay(d);
  Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)4);
  uint32_t r = Wire.read();
  // assembles temp into highest 16 bits, humidity into lowest 16 bits
  r <<= 8;
  r |= Wire.read();
  r <<= 8;
  r |= Wire.read();
  r <<= 8;
  r |= Wire.read();
  //Serial.println(r, HEX);
  return r;
}
