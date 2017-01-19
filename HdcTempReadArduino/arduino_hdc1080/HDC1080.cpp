#include <Wire.h>

#include "HDC1080.h"


HDC1080::HDC1080()
{
}

void HDC1080::begin(uint8_t address) {
	_address = address;
	Wire.begin();

	/*
	 Heater disabled,
	 Temperature and Humidity Measurement Resolution 14 bit
	*/
	Wire.beginTransmission(_address);
	Wire.write(CONFIGURATION);
	Wire.write(0x0);
	Wire.write(0x0);
	Wire.endTransmission();

}


//float HDC1080::getHumidity(void) {
//  // reads both temp and humidity but masks out temp in highest 16 bits
//  // does not set private variable
//  float hum = (read32(HDC1080_TEMP, 20) & 0xFFFF);
//
//  hum /= 65536;
//  hum *= 100;
//
//  return hum;
//}
//
//float HDC1080::getTemperatureCelcius(void) {
//  // getter function to access private temp variable
//  uint32_t rt = read32(HDC1080_TEMP, 20) >> 16; // relative temperature
//  float (temp = (rt));        // convert to temp first
//  temp /= 65536;
//  temp *= 165;
//  temp -= 40;
//  return temp ;
//}
//
//float HDC1080::getTemperatureFahrenheit(void) {
//  // getter function to access private temp variable
//  float celcius = getTemperatureCelcius();
//  float fahrenheit = (celcius * 9/5) + 32;
//  return fahrenheit;
//}
//
float HDC1080::readT() {
	return readTemperature();
}

float HDC1080::readTemperature() {
	uint16_t rawT = readData(TEMPERATURE);
	return (((rawT / pow(2, 16)) * 165 - 40)* 9/5) + 32;
}

float HDC1080::readH() {
	return readHumidity();
}

float HDC1080::readHumidity() {
	uint16_t rawH = readData(HUMIDITY);
	return (rawH / pow(2, 16)) * 100;
}

uint16_t HDC1080::readManufacturerId() {
	return readData(MANUFACTURER_ID);
}

uint16_t HDC1080::readDeviceId() {
	return readData(DEVICE_ID);
}

uint16_t HDC1080::readData(uint8_t pointer) {
	Wire.beginTransmission(_address);
	Wire.write(pointer);
	Wire.endTransmission();

	delay(10);
	Wire.requestFrom(_address, (uint8_t)2);

	byte msb = Wire.read();
	byte lsb = Wire.read();

	return msb << 8 | lsb;
}
