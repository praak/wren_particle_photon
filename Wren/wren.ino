#include "Hdc1080.h"
#include "Adafruit_GFX.h"
#include "Adafruit_PCD8544.h"

#define INPUT_SIZE 10

// Set up display using Adafruit's library
Adafruit_PCD8544 display = Adafruit_PCD8544(A0, A2, A1);
// Set up Temperature/Humidity sensor using Adafruit's library
Hdc1080 hdc = Hdc1080();

char publishString[512];
char dbString[512];

int currentWallTemp, RemoteId_1, Temp_1, RemoteId_2, Temp_2, heater = D6, airCon = D5, fan = D3, tempUpButton = D4, tempDownButton = D2, currentTemp = 72, HVACcontrol = 0, BattCheck1 = 15, BattCheck2 = 15,  currentSetTemp = 72, isrTimer = 0;

const int acOn = 1, heatOn = 2;
String BattStatus1, BattStatus2, statusHVAC;

bool displayChanged = true;

// Timed functions
Timer batteryCheckTimer(1000, batteryCheck);
Timer androidDataPublishTimer(5000, androidDataPublish);
Timer mongoDBPublishTimer(15000, mongoDBPublish);

void setup() {
        // Start the display
        display.begin();
        display.clearDisplay();
        display.println("Starting Up...");
        display.setContrast(65);
        display.display();

        // Begin timed functions
        batteryCheckTimer.start();
        androidDataPublishTimer.start();
        mongoDBPublishTimer.start();

        // Instantiate Particle articles
        Particle.function("setTemp",setCurrentSetTemperature);
        Particle.variable("Data", publishString);

        // Begin Serial for the XBee modules
        Serial1.begin(9600);

        // Set up hardware
        pinMode(heater, OUTPUT);
        pinMode(airCon, OUTPUT);
        pinMode(fan, OUTPUT);
        pinMode(tempUpButton, INPUT);
        pinMode(tempDownButton, INPUT);

        // Set up interrupts for controlling temperature override
        attachInterrupt(digitalPinToInterrupt(tempUpButton), isrTempUp, RISING);
        attachInterrupt(digitalPinToInterrupt(tempDownButton), isrTempDown, RISING);

        // Set up temperature/humidity sensor, take initial reading
        hdc.begin(0x40);
        delay(15);
        currentWallTemp = int(hdc.getTemperatureFahrenheit());
}

void loop() {
        checkRemoteSensors();
        switchHvacControl();

        if (displayChanged)
        {
                updateDisplay();
                // reset display changed flag
                displayChanged = false;
        }

        isrTimer = 0;
}

void checkRemoteSensors() {
        // TODO: Include the source link that this tutorial code came from
        // Get next command from Serial (add 1 for final 0)
        int remoteId;
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
                        remoteId = atoi(sensorData);
                        ++separator;
                        remoteTemp = atoi(separator);
                        // Identifies which sensor it is and sets the input temperature to the appropriate variable
                        if(remoteId == 1)
                        {
                                RemoteId_1 = 1;
                                Temp_1 = remoteTemp;
                                BattCheck1 = 0;
                        }
                        if(remoteId == 2)
                        {
                                RemoteId_2 = 2;
                                Temp_2 = remoteTemp;
                                BattCheck2 = 0;
                        }
                }
                // Find the next set of sensorData in input string
                sensorData = strtok(0, "\n");
        }
}

void switchHvacControl() {
        // Assume '999' indicates sensor turned off, otherwise it's passing a correct Fahrenheit value
        // Plus one for wall unit temp sensor
        int numRunningSensors = (Temp_1 == 999 ? 0 : 1) + (Temp_2 == 999 ? 0 : 1) + 1;
        // Measured temp is the average of all available sensors
        int measuredTemp = int(((Temp_1 == 999 ? 0 : Temp_1) + (Temp_2 == 999 ? 0 : Temp_2) + currentWallTemp) / numRunningSensors);

        //
        if (currentTemp != measuredTemp)
        {
                currentTemp = measuredTemp;
                displayChanged = true;
        }

        // Case statements for determining whether or not to turn on the AC/Heat/Fan
        switch (HVACcontrol) {
        case acOn:   // Turns on the AC
        {
                (currentTemp > currentSetTemp) ? setState(airCon, HIGH, fan, HIGH, "AC on - ") : HVACcontrol = 0;
        }
        break;
        case heatOn:   // Turns on the Heat
        {
                (currentTemp < currentSetTemp) ? setState(heater, HIGH, fan, HIGH, "Heat on - ") : HVACcontrol = 0;
        }
        break;
        // Nothing is on by default, turns heater/AC/fan off
        default:
        {
                setState(airCon, LOW, heater, LOW, "Idle - ");
                if (currentTemp >= currentSetTemp+2)
                {
                        HVACcontrol = acOn;
                }
                else if (currentTemp <= currentSetTemp-2)
                {
                        HVACcontrol = heatOn;
                }
        }
        break;
        }
}

bool setState(int pin1, int state1, int pin2, int state2, String status) {
        digitalWrite(pin1, state1);
        digitalWrite(pin2, state2);
        statusHVAC = status;
        displayChanged = true;
}

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

void isrTempUp()
{
        if ((isrTimer%500) == 0)
        {
                currentSetTemp = currentSetTemp + 1;
                isrTimer = isrTimer + 1;
                displayChanged = true;
        }
}
// if down button is pressed, then decrease temp by 1
void isrTempDown()
{
        if ((isrTimer%500) == 0)
        {
                currentSetTemp = currentSetTemp - 1;
                isrTimer = isrTimer + 1;
                displayChanged = true;
        }
}
/**
   Use this function to publish the json string necessary for the app
 */
void androidDataPublish () {
        sprintf(publishString,"{\"WallTemp\": \"%d\",\"RSensors\":[{\"RemoteId\": \"%d\",\"Temp\":\"%s\",\"BattStatus\": \"%s\"},{\"RemoteId\": \"%d\",\"Temp\": \"%s\",\"BattStatus\": \"%s\"}]}",
                currentWallTemp, RemoteId_1, (Temp_1 == 999 ? "--" : String(Temp_1)).c_str(), BattStatus1.c_str(), RemoteId_2, (Temp_2 == 999 ? "--" : String(Temp_2)).c_str(), BattStatus2.c_str());
        Particle.publish("wall_temp",String(currentWallTemp));
}
/**
   Use this function to publish data to the database
   TODO: see if there is an easier way to simplify the database inserts such that we can merge the json and db publish functions.
 */
void mongoDBPublish () {
        sprintf(dbString,"{\"WallTemp\": \"%d\",\"RemoteId1\": \"%d\",\"Temp1\":\"%s\",\"BattStatus1\": \"%s\",\"RemoteId2\": \"%d\",\"Temp2\": \"%s\",\"BattStatus2\": \"%s\", \"CurrentSetTemp\": \"%d\"}",
                currentWallTemp, RemoteId_1, (Temp_1 == 999 ? "--" : String(Temp_1)).c_str(), BattStatus1.c_str(), RemoteId_2, (Temp_2 == 999 ? "--" : String(Temp_2)).c_str(), BattStatus2.c_str(), getCurrentSetTemperature());
        /** Use 'mongoDB' event name to publish to the sensor data collection,
         * browse to https://particle.charlesscholle.com/?hour=0&minute=0&second=15
         * to view data published in the last 15 seconds
         **/
        Particle.publish("mongoDB",dbString);
}

void updateDisplay()
{
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(31,0);
        display.println("WREN");
        display.setTextSize(2);
        display.setCursor(31,8);
        display.println(currentTemp);
        display.setTextSize(1);
        display.print(statusHVAC);
        display.println(getCurrentSetTemperature());
        display.display();
}

void batteryCheck()
{
        if (BattCheck1 < 10)
        {
                BattStatus1 = "true";
                BattCheck1 = BattCheck1 + 1;
        }
        else
        {
                BattStatus1 = "false";
        }

        if (BattCheck2 < 10)
        {
                BattStatus2 = "true";
                BattCheck2 = BattCheck2 + 1;
        }
        else
        {
                BattStatus2 = "false";
        }


        BattStatus1 == "false" ? Temp_1 = 999 : 0;
        BattStatus2 == "false" ? Temp_2 = 999 : 0;
}
