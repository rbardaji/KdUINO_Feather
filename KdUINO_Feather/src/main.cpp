#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_TCS34725.h"

// Settings
int initial_wait = 10;      // Time to wait before start the loop (in seconds)
int measures = 1;           // Number of measurements to do[1, period-1]
int period = 60;            // Sampling period (in seconds)
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

// Vars
RTC_PCF8523 rtc;
const int chipSelect_SD = 15;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

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
    // measurements
    // Save values to SD
    delay(500);
}

// 
/*// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_PCF8523 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup () {

  Serial.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop () {
    DateTime now = rtc.now();
    
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
    
    // calculate a date which is 7 days, 12 hours and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));
    
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
    Serial.println();
    
    Serial.println();
    delay(3000);
}*/

/*#include <Wire.h>
#include "Adafruit_TCS34725.h"

/* Example code for the Adafruit TCS34725 breakout library */

/* Connect SCL    to analog 5
   Connect SDA    to analog 4
   Connect VDD    to 3.3V DC
   Connect GROUND to common ground */
   
/* Initialise with default values (int time = 2.4ms, gain = 1x) */
// Adafruit_TCS34725 tcs = Adafruit_TCS34725();

/* Initialise with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

void setup(void) {
  Serial.begin(9600);
  
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }
  
  // Now we're ready to get readings!
}

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;
  
  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC); Serial.print(" K - ");
  Serial.print("Lux: "); Serial.print(lux, DEC); Serial.print(" - ");
  Serial.print("R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.print("C: "); Serial.print(c, DEC); Serial.print(" ");
  Serial.println(" ");
}*/