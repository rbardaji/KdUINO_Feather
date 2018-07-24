#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_TCS34725.h"
#include <ESP8266WiFi.h>

// Settings
int initial_wait = 1;       // Time to wait before start the loop (in seconds)
int measures = 60;          // Number of measurements to do[1, 59]
int period = 1;             // Sampling period (in minutes) [1, 60]
float depth = 3.0;          // Absolute depth of the device [0.1, 30] (in meters)
float lat = 0;              // Latitude
float lon = 0;              // Longitude
int sample_counter = 1;     // Counter of measurements
String name = "KduinoPro2"; // Name of the module   
String maker = "ICM-CSIC";  // Maker name
String curator = "ICM-CSIC";// Curator name
String email = "";          // Email of the curator
String sensors = "TCS34725";// List with name of used sensors "Sensor 1, ..., Sensor n"
String description = "KduinoPro2 module12 3.0m 60 measurements";
String place = "lab ICM";   // Text with place of deployment
                            // Units of the measurements "Unit 1, ..., Unit n"
String units = "counts, counts, counts, counts";

// Contants
#define BAUDRATE 9600
#define REDLED 0
#define BLUELED 2
// #define TCS34725LED 14
#define BATPIN A0 // Power management

// Communication protocol
#define UPDATE_RTC 'T'
// Vars
RTC_PCF8523 rtc;
const int chipSelect_SD = 15;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_1X);
uint16_t r, g, b, c; 
float battery_level;
DateTime now;

// Function declaration
void measure_TCS34725();
void serial_data();
void serial_metadata();
void serial_header();
void serial_date();
void save_data();
void save_new_line();
void save_date();
void save_metadata();
void save_header();
void actions();
void update_rtc();

void setup () {
    // CONFIGURATION
    
    // Red Led
    pinMode(REDLED, OUTPUT);
    digitalWrite(REDLED, HIGH);
    pinMode(BLUELED, OUTPUT);
    digitalWrite(BLUELED, LOW);
    // pinMode(TCS34725LED, OUTPUT);
    // digitalWrite(TCS34725LED, LOW);

    // Wifi (disable) 
    WiFi.mode(WIFI_OFF);

    // Serial
    Serial.begin(BAUDRATE);
    Serial.println("");
    // RTC
    Serial.print("Initializing RTC.");
    if (! rtc.begin()) {
        digitalWrite(REDLED, LOW);
        while (1){
            Serial.println("Couldn't find RTC");
            delay(10000);
        }
    }
    Serial.println(" Done.");
    if (! rtc.initialized()) {
        Serial.println("RTC is NOT running!");
        // following line sets the RTC to the date & time this sketch was compiled
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
    }
    
    // SD
    Serial.print("Initializing SD card.");
    pinMode(SS, OUTPUT);
    if (!SD.begin(chipSelect_SD)) {
        digitalWrite(REDLED, LOW);
        while (1){
            Serial.println("Card failed, or not present");
            delay(10000);
        }
    }
    Serial.println(" Done.");
    
    // TCS34725
    Serial.print("Initializing TCS34725.");
    if (tcs.begin()) {
        tcs.clearInterrupt();
        Serial.println(" Done.");
    } else {
        digitalWrite(REDLED, LOW);
        while (1){
            Serial.println("No TCS34725 found");
            delay(10000);
        }
    }

    //Update time of RTC
    Serial.print("Write now the actual date and time (YYYYMMDDhhmmss)");
    update_rtc();
    
    // Read setting from settings.txt of the SD

    // Initial wait
    Serial.print("Waiting ");
    Serial.print(initial_wait);
    Serial.println(" seconds");
    delay(initial_wait*1000);
    
    // Read time
    // now = rtc.now();
    serial_date();

    // Write metadata and header into file.txt
    save_metadata();
    save_header();

    // Send metadata and header through serial communication
    serial_metadata();
    serial_header();

    digitalWrite(BLUELED, HIGH);
}

void loop () {
    // Read time
    now = rtc.now();
    
    // Check if it is time to measure
    if (now.minute() % period == 0){
        if (now.second() == 0){
            // Save time
            save_date();
            serial_date();
            
            // Measurement
            for (int i = 0; i < measures; i++) {
                measure_TCS34725();
                save_data();
                serial_data();
            }
            save_new_line();
            // Just to check time
            now = rtc.now();
            serial_date();
        }
    }
    actions();
    delay(500);
}

////////////////////////////////////////////////////////////
//////////////////// MEASUREMENTS //////////////////////////
////////////////////////////////////////////////////////////

void measure_TCS34725(){
        
    uint16_t r_, g_, b_, c_;
    // Measure
    tcs.getRawData(&r_, &g_, &b_, &c_);
    
    // Save data into global variables
    r = r_;
    g = g_;
    b = b_;
    c = c_;
}

////////////////////////////////////////////////////////////
//////////////////// SERIAL COMMUNICATION //////////////////
////////////////////////////////////////////////////////////

void serial_date(){
    /*It sends the data throught the serial  communication*/
    Serial.print(now.year(), DEC);
    Serial.print("-");
    Serial.print(now.month(), DEC);
    Serial.print("-");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.println(now.second(), DEC);
}

void serial_data(){
    /*It sends the data throught the serial  communication*/
    Serial.print(" ");
    Serial.print(r, DEC);
    Serial.print(" ");
    Serial.print(g, DEC);
    Serial.print(" ");
    Serial.print(b, DEC);
    Serial.print(" ");
    Serial.print(c, DEC);
    Serial.println("");
}

void serial_metadata(){
    /*It sends the metadata info throught the serial  communication*/
    Serial.println("METADATA");
    Serial.print("initial_wait: ");
    Serial.println(initial_wait, DEC);
    Serial.print("measures: ");
    Serial.println(measures, DEC);
    Serial.print("period: ");
    Serial.println(period, DEC);
    Serial.print("depth: ");
    Serial.println(depth);
    Serial.print("lat: ");
    Serial.println(lat);
    Serial.print("lon: ");
    Serial.println(lon);
    Serial.print("timestamp: ");
    Serial.print(now.year(), DEC);
    Serial.print("-");
    Serial.print(now.month(), DEC);
    Serial.print("-");
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(":");
    Serial.print(now.minute(), DEC);
    Serial.print(":");
    Serial.println(now.second(), DEC);
    Serial.print("sample_counter: ");
    Serial.println(sample_counter);
    Serial.print("name: ");
    Serial.println(name);
    Serial.print("maker: ");
    Serial.println(maker);
    Serial.print("curator: ");
    Serial.println(curator);
    Serial.print("email: ");
    Serial.println(email);
    Serial.print("sensors: ");
    Serial.println(sensors);
    Serial.print("description: ");
    Serial.println(description);
    Serial.print("place: ");
    Serial.println(place);
    Serial.print("units: ");
    Serial.println(units);
}

void serial_header(){
    /*It sends the header info of the datathrought the serial
    communication*/
    Serial.println("DATA");
    Serial.println("DATE HOUR RED GREEN BLUE CLEAR");
}

////////////////////////////////////////////////////////////
//////////////////// SD MANAGEMENT /////////////////////////
////////////////////////////////////////////////////////////

void save_data(){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File data_file = SD.open("data.txt", FILE_WRITE);
    if (! data_file) {
        digitalWrite(REDLED, LOW);
        // Wait forever since we cant write data
        while (1){
            Serial.println("error opening data.txt");
            delay(10000);
        }
    }
    // Save data
    data_file.print(" "); data_file.flush();
    data_file.print(r, DEC); data_file.flush();
    data_file.print(" "); data_file.flush();
    data_file.print(g, DEC); data_file.flush();
    data_file.print(" "); data_file.flush();
    data_file.print(b, DEC); data_file.flush();
    data_file.print(" "); data_file.flush();
    data_file.print(c, DEC); data_file.flush();
    // Close dataFile
    data_file.close();
}

void save_date(){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File data_file = SD.open("data.txt", FILE_WRITE);
    if (! data_file) {
        digitalWrite(REDLED, LOW);
        // Wait forever since we cant write data
        while (1){
            Serial.println("error opening data.txt");
            delay(10000);
        }
    }
    // Save data
    data_file.print(now.year(), DEC); data_file.flush();
    data_file.print("-"); data_file.flush();
    data_file.print(now.month(), DEC); data_file.flush();
    data_file.print("-"); data_file.flush();
    data_file.print(now.day(), DEC); data_file.flush();
    data_file.print(" "); data_file.flush();
    data_file.print(now.hour(), DEC); data_file.flush();
    data_file.print(":"); data_file.flush();
    data_file.print(now.minute(), DEC); data_file.flush();
    data_file.print(":"); data_file.flush();
    data_file.print(now.second(), DEC); data_file.flush();
    // Close dataFile
    data_file.close();
}

void save_new_line(){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File data_file = SD.open("data.txt", FILE_WRITE);
    if (! data_file) {
        digitalWrite(REDLED, LOW);
        // Wait forever since we cant write data
        while (1){
            Serial.println("error opening data.txt");
            delay(10000);
        }
    }
    // Save new line
    data_file.println(""); data_file.flush();
    // Close dataFile
    data_file.close();
}

void save_metadata(){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File data_file = SD.open("data.txt", FILE_WRITE);
    if (! data_file) {
        digitalWrite(REDLED, LOW);
        // Wait forever since we cant write data
        while (1){
            Serial.println("error opening data.txt");
            delay(10000);
        }
    }
    // Save metadata
    data_file.println("METADATA"); data_file.flush();
    data_file.print("initial_wait: "); data_file.flush();
    data_file.println(initial_wait, DEC); data_file.flush();
    data_file.print("measures: "); data_file.flush();
    data_file.println(measures, DEC); data_file.flush();
    data_file.print("period: "); data_file.flush();
    data_file.println(period, DEC); data_file.flush();
    data_file.print("depth: "); data_file.flush();
    data_file.println(depth); data_file.flush();
    data_file.print("lat: "); data_file.flush();
    data_file.println(lat); data_file.flush();
    data_file.print("lon: "); data_file.flush();
    data_file.println(lon); data_file.flush();
    data_file.print("timestamp: "); data_file.flush();
    data_file.print(now.year(), DEC); data_file.flush();
    data_file.print("-"); data_file.flush();
    data_file.print(now.month(), DEC); data_file.flush();
    data_file.print("-"); data_file.flush();
    data_file.print(now.day(), DEC); data_file.flush();
    data_file.print(" "); data_file.flush();
    data_file.print(now.hour(), DEC); data_file.flush();
    data_file.print(":"); data_file.flush();
    data_file.print(now.minute(), DEC); data_file.flush();
    data_file.print(":"); data_file.flush();
    data_file.println(now.second(), DEC); data_file.flush();
    data_file.print("sample_counter: "); data_file.flush();
    data_file.println(sample_counter); data_file.flush();
    data_file.print("name: "); data_file.flush();
    data_file.println(name); data_file.flush();
    data_file.print("maker: "); data_file.flush();
    data_file.println(maker); data_file.flush();
    data_file.print("curator: "); data_file.flush();
    data_file.println(curator); data_file.flush();
    data_file.print("email: "); data_file.flush();
    data_file.println(email); data_file.flush();
    data_file.print("sensors: "); data_file.flush();
    data_file.println(sensors); data_file.flush();
    data_file.print("description: "); data_file.flush();
    data_file.println(description); data_file.flush();
    data_file.print("place: "); data_file.flush();
    data_file.println(place); data_file.flush();
    data_file.print("units: "); data_file.flush();
    data_file.println(units); data_file.flush();
    // Close dataFile
    data_file.close();
}

void save_header(){
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File data_file = SD.open("data.txt", FILE_WRITE);
    if (! data_file) {
        digitalWrite(REDLED, LOW);
        // Wait forever since we cant write data
        while (1){
            Serial.println("error opening data.txt");
            delay(10000);
        }
    }
    // Save header
    data_file.println("DATA"); data_file.flush();
    data_file.println("DATE HOUR RED GREEN BLUE CLEAR");
    data_file.flush();
    // Close dataFile
    data_file.close();
}


///////////////////////////////////////////
///////////// ACTIONS /////////////////////
///////////////////////////////////////////

void actions() {
    int inByte = 0; // incomming serial byte
    if (Serial.available() > 0) {
        // get incoming byte:
        inByte = Serial.read();
        if (inByte == UPDATE_RTC){
            update_rtc();
        }
    }
}

void update_rtc() {
    int watchdog_time = 100; // To do it not blocking
    int inByte = 0;
    
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    for (int i=0; i<watchdog_time; i++) {
        if (Serial.available() > 13) { // YYYYMMDDhhmmss (14 bytes to receive)
            for (int date_position = 0; date_position < 14; date_position++) {
                // get incoming byte
                inByte = Serial.read();
                switch (date_position) {
                    case 0:
                        year += (inByte - '0')*1000;
                        break;
                    case 1:
                        year += (inByte - '0')*100;
                        break;
                    case 2:
                        year += (inByte - '0')*10;
                        break;
                    case 3:
                        year += (inByte - '0');
                        break;
                    case 4:
                        month += (inByte - '0')*10;
                        break;
                    case 5:
                        month += (inByte - '0');
                        break;
                    case 6:
                        day += (inByte - '0')*10;
                        break;
                    case 7:
                        day += (inByte - '0');
                        break;
                    case 8:
                        hour += (inByte - '0')*10;
                        break;
                    case 9:
                        hour += (inByte - '0');
                        break;
                    case 10:
                        minute += (inByte - '0')*10;
                        break;
                    case 11:
                        minute += (inByte - '0');
                        break;
                    case 12:
                        second += (inByte - '0')*10;
                        break;
                    case 13:
                        second += (inByte - '0');
                        break;
                }
            }
            rtc.adjust(DateTime(year, month, day, hour, minute, second));
            // Read time
            now = rtc.now();
            serial_date();
            return;
        }
        else delay(1000);
    }
}
