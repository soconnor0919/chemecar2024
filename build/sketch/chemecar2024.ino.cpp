#include <Arduino.h>
#line 1 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/chemecar2024.ino"
#include <Adafruit_AS7341.h>
#include "state.h"

Adafruit_AS7341 as7341;

state SYS_STATE;

bool start = false;
bool done = false;

long startTime = 0;
// 1000, 100
// 500, 50
uint16_t START_THRESHOLD = 1000;
uint16_t END_THRESHOLD = 100;

#line 17 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/chemecar2024.ino"
void setup();
#line 45 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/chemecar2024.ino"
void loop();
#line 92 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/chemecar2024.ino"
void printData();
#line 100 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/chemecar2024.ino"
void printResults();
#line 17 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/chemecar2024.ino"
void setup()
{
    // Set system state to NOT_READY
    SYS_STATE = NOT_READY;

    Serial.begin(115200);

    // Wait for communication with the spectrometer sensor
    if (!as7341.begin())
    {
        Serial.println("Could not find spectrometer sensor. Check your connections.");
        while (1)
        {
            delay(10);
        }
    }

    // Configure spectrometer sensor
    as7341.setATIME(100);
    as7341.setASTEP(999);
    as7341.setGain(AS7341_GAIN_256X);

    // Set system state to READY
    SYS_STATE = READY;

    Serial.println("Waiting for activation...");
}

void loop()
{
    // Read all channels at the same time and store in as7341 object
    if (!as7341.readAllChannels())
    {
        Serial.println("Error reading all channels!");
        return;
    }

    // State logic
    switch (SYS_STATE)
    {
    case NOT_READY:
        // System is in an error state. This should never occur.
        Serial.println("System is in an error state. Please restart.");
        exit(1);
        break;
    case READY:
        // System is waiting for activation threshold
        if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) >= START_THRESHOLD)
        {
            SYS_STATE = RUNNING;
            startTime = millis(); // For debugging and measurement purposes
            Serial.println("Starting measurement."); 
        }
        break;
    case RUNNING:
        // System is currently running the measurement
        if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) <= END_THRESHOLD)
        {
            SYS_STATE = DONE;
            Serial.println("Measurement complete.");
            long finalTime = millis() - startTime;
            Serial.print("Time: ");
            Serial.print(finalTime / 1000.0, 3);
            Serial.print(" seconds.");
            Serial.println("");
        }
        break;
    case DONE:
        // System has completed the measurement
        Serial.println("Measurement complete.");
        exit(0);
        break;
    }
}

void printData()
{
    // Print out the stored values for each channel
    Serial.print("480nm:");
    Serial.print(as7341.getChannel(AS7341_CHANNEL_480nm_F3));
    Serial.println("");
}

void printResults()
{
    Serial.println("Measurement complete.");
    long finalTime = millis() - startTime;
    Serial.print("Time: ");
    Serial.print(finalTime / 1000.0, 3);
    Serial.print(" seconds.");
    Serial.println("");
}
#line 1 "/Users/soconnor/Documents/Projects/cheg230/chemecar2024/calibrationTest.ino"
// /* This example will read all channels from the AS7341 and print out reported values */

// #include <Adafruit_AS7341.h>

// Adafruit_AS7341 as7341;

// bool start = false;
// bool done = false;

// long startTime = 0;
// // 1000, 100
// // 500, 50
// static long START_THRESHOLD = 1000;
// static long END_THRESHOLD = 100;

// void setup() {
//   Serial.begin(115200);
//   Serial.println("Initialization");

//   // Wait for communication with the host computer serial monitor
//   // while (!Serial) {
//   //   delay(1);
//   // }
  
//   if (!as7341.begin()){
//     Serial.println("Could not find spectrometer sensor. Check your connections.");
//     while (1) { delay(10); }
//   }

//   as7341.setATIME(100);
//   as7341.setASTEP(999);
//   as7341.setGain(AS7341_GAIN_256X);

//   // Clear terminal
//   for (int i; i < 20; i++) {
//     Serial.println("");
//   }

//   // Welcome message
//   Serial.println("-------------------------");
//   Serial.println("|   LUMINOL REACTION    |");
//   Serial.println("| THRESHOLD MEASUREMENT |");
//   Serial.println("-------------------------");
//   Serial.println("");

//   Serial.println("Waiting for activation...");

//   // FOR TESTING
//   // activateMeasurement();
// }

// void loop() {
//   // Read all channels at the same time and store in as7341 object
//   if (!as7341.readAllChannels()){
//     Serial.println("Error reading all channels!");
//     return;
//   }

//   if (!start && !done) {
//     activateMeasurement();
//     printData();
//   }

//   if (start && !done) {
//     printData();
//     if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) <= END_THRESHOLD) {
//       done = true;
//       printResults();
//     }
//   }

//   // printData();
// }

// void printData() {
//   // Print out the stored values for each channel
//   Serial.print("480nm:");
//   Serial.print(as7341.getChannel(AS7341_CHANNEL_480nm_F3));
//   Serial.println("");
// }

// void printResults() {
//   Serial.println("Measurement complete.");
//   long finalTime = millis() - startTime;
//   Serial.print("Time: ");
//   Serial.print(finalTime / 1000.0, 3);
//   Serial.print(" seconds.");
//   Serial.println("");
// }

// void activateMeasurement() {
//   if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) >= START_THRESHOLD) {
//     startTime = millis();
//     start = true;
//     Serial.println("Starting measurement.");
//   } 
// }
