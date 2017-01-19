#ifndef _HDC1080_h

#define _HDC1080_h
#include <Arduino.h>

typedef enum {
	TEMPERATURE = 0x00,
	HUMIDITY = 0x01,
	CONFIGURATION = 0x02,
	MANUFACTURER_ID = 0xFE,
	DEVICE_ID = 0xFF,
	SERIAL_ID_FIRST = 0xFB,
	SERIAL_ID_MID = 0xFC,
	SERIAL_ID_LAST = 0xFD,
} HDC1080_Pointers;

class HDC1080 {
public:
	HDC1080();

	void begin(uint8_t address);
	uint16_t readManufacturerId(); // 0x5449 ID of Texas Instruments
	uint16_t readDeviceId(); // 0x1050 ID of the device

//	float getHumidity();
//	float getTemperatureCelcius();
//	float getTemperatureFahrenheit();
	float readTemperature();
	float readHumidity();

	float readT(); // short-cut for readTemperature
	float readH(); // short-cut for readHumidity

private:
	uint8_t _address;
	uint16_t readData(uint8_t pointer);

};

#endif
