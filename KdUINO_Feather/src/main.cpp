#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_TCS34725.h"

// Settings
int initial_wait = 5;       // Time to wait before start the loop (in seconds)
int measures = 1;           // Number of measurements to do[1, 59]
int period = 1;             // Sampling period (in minutes) [1, 60]
float depth = 0.3;          // Absolute depth of the device [0.1, 30] (in meters)
float lat = 0;              // Latitude
float lon = 0;              // Longitude
String timestamp = "2018/01/01 00:00:00";
int sample_counter = 1;     // Counter of measurements
String name = "KdUINOPro";  // Name of the module   
String maker = "ICM-CSIC";  // Maker name
String curator = "ICM-CSIC";// Curator name
String email = "";          // Email of the curator
String sensors = "TCS34725";// List with name of used sensors "Sensor 1, ..., Sensor n"
String description = "Test prototype";
String place = "lab ICM";   // Text with place of deployment
String units = "";          // Units of the measurements "Unit 1, ..., Unit n"

// Contants
#define BAUDRATE 9600
#define REDLED 0
#define BLUELED 2
#define TCS34725LED 14
#define BATPIN A0 // Power management

// Vars
RTC_PCF8523 rtc;
const int chipSelect_SD = 15;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);
uint16_t r, g, b, c, colorTemp, lux;
float battery_level;

// Function declaration
void measure_TCS34725();
void measure_battery();
void serial_data();
void save_data();

void setup () {
    // CONFIGURATION
    
    // Red Led
    pinMode(REDLED, OUTPUT);
    digitalWrite(REDLED, HIGH);
    pinMode(BLUELED, OUTPUT);
    digitalWrite(BLUELED, LOW);
    
    // Serial
    Serial.begin(BAUDRATE);
    
    // RTC
    Serial.print("Initializing RTC.");
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        digitalWrite(REDLED, LOW);
        while (1);
    }
    Serial.println(" Done.");
    
    // SD
    Serial.print("Initializing SD card.");
    pinMode(SS, OUTPUT);
    if (!SD.begin(chipSelect_SD)) {
        Serial.println("Card failed, or not present");
        digitalWrite(REDLED, LOW);
        while (1);
    }
    Serial.println(" Done.");
    
    // TCS34725
    Serial.print("Initializing TCS34725.");
    if (tcs.begin()) {
        Serial.println(" Done.");
    } else {
        Serial.println("No TCS34725 found");
        digitalWrite(REDLED, LOW);
        while (1);
    }
    
    // Read setting from settings.txt of the SD

    // Initial wait
    delay(initial_wait*1000);
    
    digitalWrite(BLUELED, HIGH);
}

void loop () {
    // Read time
    DateTime now = rtc.now();
    
    // Check if it is time to measure
    if (now.minute() % period == 0){
        if (now.second() == 0){
            // Measurement
            measure_TCS34725();
            measure_battery();
            // Send data to serial comunication
            serial_data();
            // Save into SD card
            save_data();
        }
    }
    delay(500);
}

void measure_TCS34725(){
    tcs.getRawData(&r, &g, &b, &c);
    colorTemp = tcs.calculateColorTemperature(r, g, b);
    lux = tcs.calculateLux(r, g, b);
}

void measure_battery(){
    // read the battery level from the analog in pin.
    // analog read level is 10 bit 0-1023 (0V-1V).
    // our 1M & 220K voltage divider takes the max
    // lipo value of 4.2V and drops it to 0.758V max.
    // this means our min analog read value should be 580 (3.14V)
    // and the max analog read value should be 774 (4.2V).
    battery_level = analogRead(BATPIN);
    // convert battery level to percent
    battery_level = map(battery_level, 580, 774, 0, 100);
}

void serial_data(){
    Serial.print(r, DEC);
    Serial.print(" ");
    Serial.print(g, DEC);
    Serial.print(" ");
    Serial.print(b, DEC);
    Serial.print(" ");
    Serial.print(c, DEC);
    Serial.print(" ");
    Serial.print(lux);
    Serial.print(" ");
    Serial.print(colorTemp);
    Serial.print(" ");
    Serial.print(battery_level);
    Serial.println("");
}

void save_data(){
    Serial.print("Estoy guardanto");
}