/*SYSTEM_MODE(AUTOMATIC);*/

// for the HDC1080 temp_humid sensor
#include "Hdc1080.h"
#include "Adafruit_GFX.h"
#include "Adafruit_PCD8544.h"


// (FAST) SOFTWARE SPI
// pin A3 - Serial clock out (SCLK) - User defined
// pin A5 - Serial data out (DIN/MOSI) - User defined
// pin D2 - Data/Command select (D/C) - User defined
// pin A2 - LCD chip select (CS) - User defined
// pin D3 - LCD reset (RST) - User defined
// Adafruit_PCD8544(SCLK, DIN, DC, CS, RST)
//Adafruit_PCD8544 display = Adafruit_PCD8544(A3, A5, D2, A2, D3);

// HARDWARE SPI
// pin A3 - Serial clock out (SCLK)
// pin A5 - Serial data out (DIN/MOSI)
// pin D2 - Data/Command select (D/C) - User defined
// pin A2 - LCD chip select (CS/SS) - User defined
// pin D3 - LCD reset (RST) - User defined
// Adafruit_PCD8544(CS, DC, RST)

// Originally D2 and D3 were used.
// Adafruit_PCD8544 display = Adafruit_PCD8544(SS, D2, D3);
Adafruit_PCD8544 display = Adafruit_PCD8544(A0, A2, A1);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

static const unsigned char logo16_glcd_bmp[] =
{ 0B00000000, 0B11000000,
  0B00000001, 0B11000000,
  0B00000001, 0B11000000,
  0B00000011, 0B11100000,
  0B11110011, 0B11100000,
  0B11111110, 0B11111000,
  0B01111110, 0B11111111,
  0B00110011, 0B10011111,
  0B00011111, 0B11111100,
  0B00001101, 0B01110000,
  0B00011011, 0B10100000,
  0B00111111, 0B11100000,
  0B00111111, 0B11110000,
  0B01111100, 0B11110000,
  0B01110000, 0B01110000,
  0B00000000, 0B00110000 };

Hdc1080 hdc = Hdc1080();

int count;
unsigned long lastTime = 0UL;
char publishString[256];

int remoteTemp(int posVal, int negVal)
{
  posVal = posVal * 0.00322265625;
  negVal = negVal * 0.00322265625;
  int voltage = posVal - negVal;
  int degreesC = -5.3546*voltage*voltage + 31.279*voltage + 21.531;
  int degreesF = degreesC * (9.0/5.0) + 32.0;
  return degreesF;
}

void setup() {
  Particle.variable("Data", publishString);
  Serial.begin(9600);
  Serial1.begin(9600);
  /*Serial.println("beginning of setup");*/
  /*delay(5000);*/
  display.begin();
  //init done

  display.setContrast(65);
  /*delay(2000);*/
  display.display();
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("WREN");
  delay(200);
  display.display();

  hdc.begin(0x40);
  delay(15);  // giving delay time for the chip to initialize

  /*Serial.println("end of setup");*/
  count=0;
}

void loop() {
  /*Serial.println("beginning of loop");*/

unsigned long now = millis();
// Working code to get temp and print on display

  float tempC = hdc.getTemperatureFahrenheit();
  int cc = int(tempC);
  Particle.publish("wall_temp",Serial1.readStringUntil('\n'));
  Serial.flush();
  /*Serial.println(tempF);*/
  delay(500);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(15,0);
  display.println("WREN");
  display.setTextSize(1);
  display.println(tempC);
  display.println(cc);
  display.println(Serial1.readStringUntil('\n'));
  /*int ndex = 0;*/
  /*int myInts[2];
  int done = 0;*/
// this may be alright now, but wont be able to work the best when reading with slow and such.
// basically it might not be able to take all the values in one run. . not sure.
  /*while (done = 0)
  {
    if (Serial1.readStringUntil('\n').toInt() == 0)
    {
      myInts[0] = Serial1.readStringUntil('\n').toInt();
      myInts[1] = Serial1.readStringUntil('\n').toInt();
      done = 1;
    }
    else
    {
      done = 0;
    }
  }*/
  /*int tempFF = remoteTemp(myInts[0],myInts[1]);*/
  /*Serial.println(myInts[0]);
  Serial.println(myInts[1]);*/
  /*Serial.println(tempFF);*/
  /*display.println(tempFF);*/
  delay(500);
  display.display();

  jsonPublish(72,1,744,2,72);
          /*sprintf(publishString,"{\"Hours\": %u, \"Minutes\": %u, \"Seconds\": %u}",hours,min,sec);*/

}

void jsonPublish (int WallTemp , int RemoteId_1, int Temp_1, int RemoteId_2, int Temp_2) {
  sprintf(publishString,"{\"WallTemp\": \"70\",\"RSensors\":[{\"RemoteId\": \"%d\",\"Temp\":\"%d\",\"BattStatus\": \"true\"},{\"RemoteId\": \"%d\",\"Temp\": \"%d\",\"BattStatus\": \"false\"}]}",
      RemoteId_1, Temp_1, RemoteId_2, Temp_2);
  Particle.publish("Data",publishString);
}
// might need this for interupts ? potentially.
/*void connect() {
  if(Particle.connected() == false){
    Particle.connect();
  }
}*/
