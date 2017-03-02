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

// Calculate based on max input size expected for one command
#define INPUT_SIZE 10
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
int RemoteId_1;
int Temp_1;
int RemoteId_2;
int Temp_2;
int heater = D6;

/*int remoteTemp(int posVal, int negVal)
{
  posVal = posVal * 0.00322265625;
  negVal = negVal * 0.00322265625;
  int voltage = posVal - negVal;
  int degreesC = -5.3546*voltage*voltage + 31.279*voltage + 21.531;
  int degreesF = degreesC * (9.0/5.0) + 32.0;
  return degreesF;
}*/

void setup() {
  Particle.variable("Data", publishString);
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(heater, OUTPUT);
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
  /*Particle.publish("wall_temp",Serial1.readStringUntil('\n'));*/
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
  digitalWrite(heater, HIGH);
  /*int RemoteData = display.println(Serial1.readStringUntil('\n'));
  int remoteTemp = display.println(Serial1.readStringUntil('\n'));*/

  // Get next command from Serial (add 1 for final 0)
  int RemoteId;
  int remoteTemp;
  char input[INPUT_SIZE + 1];
  byte size = Serial1.readBytes(input, INPUT_SIZE);
  // Add the final 0 to end the C string
  input[size] = 0;

  // Read each command pair
  char* command = strtok(input, " \n");
  while (command != 0)
  {
      // Split the command in two values
      char* separator = strchr(command, ':');
      if (separator != 0)
      {
          // Actually split the string in 2: replace ':' with 0
          *separator = 0;
          RemoteId = atoi(command);
          ++separator;
          remoteTemp = atoi(separator);
          if(RemoteId == 1)
          {
            RemoteId_1 = 1;
            Temp_1 = remoteTemp;
          }
          if(RemoteId == 2)
          {
              RemoteId_2 = 2;
              Temp_2 = remoteTemp;
          }
          display.print(RemoteId);
          display.println(remoteTemp);
          // Do something with servoId and position
      }
      // Find the next command in input string
      command = strtok(0, "\n");
  }

  delay(500);
  display.display();
 jsonPublish(cc, RemoteId_1, Temp_1, RemoteId_2, Temp_2);
  /*jsonPublish(72,1,744,2,72);*/
          /*sprintf(publishString,"{\"Hours\": %u, \"Minutes\": %u, \"Seconds\": %u}",hours,min,sec);*/

}
void jsonPublish (int WallTemp , int RemoteId_1, int Temp_1, int RemoteId_2, int Temp_2) {
  sprintf(publishString,"{\"WallTemp\": \"%d\",\"RSensors\":[{\"RemoteId\": \"%d\",\"Temp\":\"%d\",\"BattStatus\": \"true\"},{\"RemoteId\": \"%d\",\"Temp\": \"%d\",\"BattStatus\": \"false\"}]}",
      WallTemp, RemoteId_1, Temp_1, RemoteId_2, Temp_2);
  Particle.publish("Data",publishString);
  Particle.publish("wall_temp",WallTemp);
}
// might need this for interupts ? potentially.
/*void connect() {
  if(Particle.connected() == false){
    Particle.connect();
  }
}*/
