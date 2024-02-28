/*
 * BUCKNELL CHEM-E-CAR TEAM PRESENTS:
 * 2024 Control System Code
 * 
 * This code is designed to run on an Adafruit Metro Mini microcontroller.
 * It is responsible for controlling the motor based off of reading 
 * spectrometer sensor values.
 * 
 * Copyright (C) 2024 Bucknell University
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * Author: Sean O'Connor (sso005@bucknell.edu)
 */

#include <Adafruit_AS7341.h>
#include "state.h"

// Relay Pin
#define RELAY_PIN 2

/*
 * MODE FLAGS:
 * These flags can be set to true or false to change the behavior of the system.
 * 
 * DESCRIPTION:
 * CALIBRATION_MODE: If true, the system will print the time it took to reach the end threshold.
 * VERBOSE_MODE: If true, the system will print the measurement values at each iteration.
 * MOTOR_ACTIVE: If true, the system will activate the motor while the reaction is running.
 * 
 * COMPETITION SETTINGS:
 * CALIBRATION_MODE: false
 * VERBOSE_MODE: false
 * MOTOR_ACTIVE: true
 */
bool CALIBRATION_MODE = false;
bool VERBOSE_MODE = false;
bool MOTOR_ACTIVE = true;

// Spectrometer sensor
Adafruit_AS7341 as7341;

// System state
state SYS_STATE;

// Store how long the reaction took for calibration purposes
unsigned long reactionTime = 0;

/*
 * MEASUREMENT THRESHOLD VALUES:
 * For the Luminol reaction, the threshold values are:
 * Start: 1000, End: 100
 * For testing purposes, the threshold values are:
 * Start: 500, End: 50
 */
uint16_t START_THRESHOLD = 1000;
uint16_t END_THRESHOLD = 100;

void setup()
{
    // Set system state to NOT_READY
    SYS_STATE = NOT_READY;

    // Set relay pin to output
    pinMode(RELAY_PIN, OUTPUT);

    // Set relay to off
    digitalWrite(RELAY_PIN, LOW);

    // Start serial communication
    Serial.begin(115200);

    // Wait for communication with the spectrometer sensor
    if (!as7341.begin())
    {
        // Sensor not found. Check connections, and wait to try again.
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
            if (CALIBRATION_MODE)
            {
                Serial.println("Starting calibration measurement.");
                reactionTime = millis(); // For debugging and measurement purposes
            }
            if (MOTOR_ACTIVE)
            {
                digitalWrite(RELAY_PIN, HIGH);
            }
        }
        break;
    case RUNNING:
        // System is currently running the measurement
        // Print measurement values if requested
        if (VERBOSE_MODE)
        {
            printData();
        }
        // Check if the measurement has reached the end threshold
        if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) <= END_THRESHOLD)
        {
            // Set system state to done, indicating reaction is complete
            SYS_STATE = DONE;
            // Print results if in calibration mode
            if (CALIBRATION_MODE)
            {
                printResults();
            }
            // Turn off the motor
            if (MOTOR_ACTIVE)
            {
                digitalWrite(RELAY_PIN, LOW);
            }
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
    long finalTime = millis() - reactionTime;
    Serial.print("Time: ");
    Serial.print(finalTime / 1000.0, 3);
    Serial.println(" seconds.");
}