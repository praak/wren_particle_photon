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

   Sensor datasheet: http://www.ti.com/lit/ds/symlink/hdc1008.pdf

 ****************************************************/
#include "application.h"

#define HDC1080_I2CADDR       0x40
#define HDC1080_TEMP          0x00
#define HDC1080_HUMID         0x01
#define HDC1080_CONFIG        0x02
#define HDC1080_CONFIG_RST    (1 << 15)
#define HDC1080_CONFIG_HEAT   (1 << 13)
#define HDC1080_CONFIG_MODE   (1 << 12)
#define HDC1080_CONFIG_BATT   (1 << 11)
#define HDC1080_CONFIG_TRES_14  0
#define HDC1080_CONFIG_TRES_11  (1 << 10)
#define HDC1080_CONFIG_HRES_14  0
#define HDC1080_CONFIG_HRES_11  (1 << 8)
#define HDC1080_CONFIG_HRES_8   (1 << 9)

#define HDC1080_SERIAL1       0xFB
#define HDC1080_SERIAL2       0xFC
#define HDC1080_SERIAL3       0xFD
#define HDC1080_MANUFID       0xFE
#define HDC1080_DEVICEID      0xFF



class Hdc1080 {
public:
        Hdc1080();
        boolean begin(uint8_t a = HDC1080_I2CADDR);
        void reset(void);
        float getTemperatureCelcius(void);      // return degrees celcius
        float getTemperatureFahrenheit(void);   // return degrees fahrenheit
        float getHumidity(void);        // added getter function to access private humidity
        boolean batteryLOW(void);      // added testing of battery voltage, useful for remote monitoring

        uint16_t read16(uint8_t a, uint8_t d=0);
        uint32_t read32(uint8_t a, uint8_t d=0);

        boolean readData(void); // not being used
        float humidity, temp;
        uint16_t battLOW;
        uint8_t _i2caddr;
};
