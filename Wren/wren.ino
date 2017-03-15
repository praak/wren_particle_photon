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
int WallTemp;
int RemoteId_1;
int Temp_1;
int RemoteId_2;
int Temp_2;
int heater = D6;
int airCon = D5;
int fan = D3;
int tempUpButton = D4;
int tempDownButton = D2;
int currentTemp = 72;
int HVACcontrol = 0;
const int acOn = 1;
const int heatOn = 2;
String BattStatus1 = " ";
String BattStatus2 = " ";
int BattCheck1 = 10;
int BattCheck2 = 10;

int currentSetTemp = 72;
// instantiate temp, later grab from EEPROM

// is used by phone app to set temp
int setCurrentSetTemperature(String temp) {
// TODO: change to store in EEPROM
currentSetTemp = temp.toInt();
return currentSetTemp;
}
// Called by the phone app. TODO: Will have to send to app via the jsonPublish
int getCurrentSetTemperature() {
// TODO: change to get from EEPROM
return currentSetTemp;
}

int isrTimer = 0;
      // if up button is pressed, then increase temp by 1


void setup() {
  Particle.function("setTemp",setCurrentSetTemperature);
  Particle.variable("Data", publishString);
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(heater, OUTPUT);
  pinMode(airCon, OUTPUT);
  pinMode(fan, OUTPUT);
  pinMode(tempUpButton, INPUT);
  pinMode(tempDownButton, INPUT);
  attachInterrupt(digitalPinToInterrupt(tempUpButton), isrTempUp, RISING);
  attachInterrupt(digitalPinToInterrupt(tempDownButton), isrTempDown, RISING);
  display.begin();
  //init done

  display.setContrast(65);
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
  count=0;
}

void loop() {

unsigned long now = millis();
// Working code to get temp and print on display

  float tempC = hdc.getTemperatureFahrenheit();
  WallTemp = int(tempC);
  /*Particle.publish("wall_temp",Serial1.readStringUntil('\n'));*/
  Serial.flush();
  //delay(500);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(31,0);
  display.println("WREN");
  display.setTextSize(2);
  display.setCursor(31,8);
  //display.println(tempWall);

  display.println(getCurrentSetTemperature());
  display.setTextSize(1);
  display.print(digitalRead(tempDownButton));
  display.println(digitalRead(tempUpButton));
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
  // char *strtok(char *str, const char *delim) breaks string str into a series of tokens using the delimiter delim
  // Separates remote sensor data into respective units
  char* sensorData = strtok(input, " \n");
  while (sensorData != 0)
  {
      // Split the remote sensor data in two values
      char* separator = strchr(sensorData, ':');
      if (separator != 0)
      {
          // Actually split the string in 2: replace ':' with 0
          *separator = 0;
          // atoi() is a C command that converts a string into an integer
          RemoteId = atoi(sensorData);
          ++separator;
          remoteTemp = atoi(separator);
          // Identifies which sensor it is and sets the input temperature to the appropriate variable
          if(RemoteId == 1)
          {
            RemoteId_1 = 1;
            Temp_1 = remoteTemp;
            BattCheck1 = 0;
          }
          if(RemoteId == 2)
          {
              RemoteId_2 = 2;
              Temp_2 = remoteTemp;
              BattCheck2 = 0;
          }
          display.print(RemoteId);
          display.println(remoteTemp);
      }
      // Find the next set of sensorData in input string
      sensorData = strtok(0, "\n");
  }

  if (BattCheck1 <= 10)
  {
  BattStatus1 = "true";
  BattCheck1 = BattCheck1 + 1;
  }
  else
  {
    BattStatus1 = "false";
  }
  if (BattCheck2 <= 10)
  {
    BattStatus2 = "true";
    BattCheck2 = BattCheck2 + 1;
  }
  else
  {
    BattStatus2 = "false";
  }
  if (BattStatus1 == "true" && BattStatus2 == "true")
  {
    currentTemp = (Temp_1+Temp_2)/2; // Removed wall temp sensor (+tempWall)
  }
  else if (BattStatus1 == "true")
  {
    currentTemp = Temp_1;
    Temp_2 = 999;
    display.println("2 Dead");
  }
  else if (BattStatus2 == "true")
  {
    currentTemp = Temp_2;
    Temp_1 = 999;
    display.println("1 Dead");
  }
  else
  {
    currentTemp = WallTemp;
    Temp_1 = 999;
    Temp_2 = 999;
    display.println("Both Dead");
  }
  // Case statements for determining whether or not to Turn on the AC/Heat/Fan
  switch (HVACcontrol) {
    case acOn: // Turns on the AC
    {
      if (currentTemp > currentSetTemp)
      {
        digitalWrite(airCon, HIGH);
        digitalWrite(fan, HIGH);
        display.print("AC on - ");
      }
      else
      {
        HVACcontrol = 0;
      }
    }
      break;
    case heatOn: // Turns on the Heat
    {
      if (currentTemp < currentSetTemp)
      {
        digitalWrite(heater, HIGH);
        digitalWrite(fan, HIGH);
        display.print("Heat On - ");
      }
      else
      {
        HVACcontrol = 0;
      }
    }
      break;
    // Nothing is on by default, turns heater/AC/fan off
    default:
    {
      digitalWrite(heater, LOW);
      digitalWrite(airCon, LOW);
      //digitalWrite(fan, LOW);
      display.print("Idle - ");
        if (currentTemp >= currentSetTemp+2)
        {
          HVACcontrol = acOn;
        }
        else if (currentTemp <= currentSetTemp-2)
        {
          HVACcontrol = heatOn;
        }
        else
        {
          HVACcontrol = 0;
        }
      }
    break;
  }
  display.println(currentTemp);
//  delay(500);
  isrTimer = 0;
  display.display();
  String Temp_1String = Temp_1 == 999? "--": String(Temp_1);
  String Temp_2String = Temp_2 == 999? "--": String(Temp_2);
 jsonPublish(currentSetTemp, RemoteId_1, Temp_1String, BattStatus1, RemoteId_2, Temp_2String, BattStatus2);
 dbPublish(currentSetTemp, RemoteId_1, Temp_1String, BattStatus1, RemoteId_2, Temp_2String, BattStatus2);
  /*jsonPublish(72,1,744,2,72);*/
          /*sprintf(publishString,"{\"Hours\": %u, \"Minutes\": %u, \"Seconds\": %u}",hours,min,sec);*/

}
void isrTempUp()
{
  if ((isrTimer%500) == 0)
  {
  currentSetTemp = currentSetTemp + 1;
  isrTimer = isrTimer + 1;
  }
}
      // if down button is pressed, then decrease temp by 1
void isrTempDown()
{
  if ((isrTimer%500) == 0)
  {
  currentSetTemp = currentSetTemp - 1;
  isrTimer = isrTimer + 1;
  }
}
/**
 Use this function to publish the json string necessary for the app
 */
void jsonPublish (int WallTemp , int RemoteId_1, String Temp_1String, String BattStatus1, int RemoteId_2, String Temp_2String, String BattStatus2) {
  sprintf(publishString,"{\"WallTemp\": \"%d\",\"RSensors\":[{\"RemoteId\": \"%d\",\"Temp\":\"%s\",\"BattStatus\": \"%s\"},{\"RemoteId\": \"%d\",\"Temp\": \"%s\",\"BattStatus\": \"%s\"}]}",
      WallTemp, RemoteId_1, Temp_1String.c_str(), BattStatus1.c_str(), RemoteId_2, Temp_2String.c_str(), BattStatus2.c_str());
  Particle.publish("Data",publishString);
  Particle.publish("wall_temp",String(WallTemp));
  Particle.publish("setTemp",String(getCurrentSetTemperature()));
  //Particle.publish("mongoDB",publishString);
  /*Particle.publish("mongodbTest",publishString);*/
}

/**
 Use this function to publish data to the database
 TODO: see if there is an easier way to simplify the database inserts such that we can merge the json and db publish functions.
 */
void dbPublish (int WallTemp , int RemoteId_1, String Temp_1String, String BattStatus1, int RemoteId_2, String Temp_2String, String BattStatus2) {
  sprintf(publishString,"{\"WallTemp\": \"%d\",\"RemoteId1\": \"%d\",\"Temp1\":\"%s\",\"BattStatus1\": \"%s\",\"RemoteId2\": \"%d\",\"Temp2\": \"%s\",\"BattStatus2\": \"%s\"}",
      WallTemp, RemoteId_1, Temp_1String.c_str(), BattStatus1.c_str(), RemoteId_2, Temp_2String.c_str(), BattStatus2.c_str());
  // Use 'mongodbTest' event name to publish to a test collection *Don't use this event unless testing
  /*Particle.publish("mongodbTest",publishString);*/

  /** Use 'mongoDB' event name to publish to the sensor data collection,
    * browse to https://particle.charlesscholle.com/?hour=0&minute=0&second=15
    * to view data published in the last 15 seconds
    */
  Particle.publish("mongoDB",publishString);
}

// might need this for interupts ? potentially.
/*void connect() {
  if(Particle.connected() == false){
    Particle.connect();
  }
}*/
