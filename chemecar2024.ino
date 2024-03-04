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
#include <Adafruit_NeoPixel.h>
#include "constants.h"
#include "hardware.h"
#include "enums/state.h"
#include "subsystems/LEDUtil.h"
#include "subsystems/LEDUtil.cpp"

/*
 * MODE FLAGS:
 * These flags can be set to true or false to change the behavior of the system.
 * 
 * DESCRIPTION:
 * CALIBRATION_MODE: If true, the system will print the time it took to reach the end threshold.
 * VERBOSE_MODE: If true, the system will print the measurement values at each iteration.
 * MOTOR_ACTIVE: If true, the system will activate the motor while the reaction is running.
 * PUMP_ACTIVE: If true, the system will activate the pump while the reaction is running.
 * LED_ACTIVE: If true, the system will activate the LED strip.
 * 
 * COMPETITION SETTINGS:
 * CALIBRATION_MODE: false
 * VERBOSE_MODE: false
 * MOTOR_ACTIVE: true
 * PUMP_ACTIVE: true
 * LED_ACTIVE: true
 */
bool CALIBRATION_MODE = false;
bool VERBOSE_MODE = false;
bool MOTOR_ACTIVE = true;
bool PUMP_ACTIVE = true;
bool LED_ACTIVE = true;

// System state
state SYS_STATE;

// Store how long the reaction took for calibration purposes
unsigned long reactionDuration = 0;

/*
 * MEASUREMENT THRESHOLD VALUES:
 * For the Luminol reaction, the threshold values are:
 * Start: 1000, End: 100
 * For testing purposes, the threshold values are:
 * Start: 500, End: 50
 */
uint16_t START_THRESHOLD = 1000;
uint16_t END_THRESHOLD = 100;

void setup() {
    // Set system state to NOT_READY
    SYS_STATE = NOT_READY;

    // Set pins to output
    pinMode(LED_PIN, OUTPUT);
    pinMode(PUMP_PIN, OUTPUT);
    pinMode(MOTOR_PIN, OUTPUT);

    // Set actuators to off
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(MOTOR_PIN, LOW);

    // Start serial communication
    Serial.begin(115200);

    // Wait for communication with the spectrometer sensor
    if (!as7341.begin()) {
        // Sensor not found. Check connections, and wait to try again.
        Serial.println("Could not find spectrometer sensor. Check your connections.");
        while (1) {
            delay(10);
        }
    }

    // Configure spectrometer sensor
    as7341.setATIME(100);
    as7341.setASTEP(999);
    as7341.setGain(AS7341_GAIN_256X);

    if (LED_ACTIVE) {
        // Initialize LEDs
        initLED();
        // Default LEDs to orange
        setLEDColor(Color::ORANGE);
    }

    // Set system state to READY
    SYS_STATE = READY;
    
    Serial.println("Waiting for activation...");
}

void loop() {
    // Read all channels at the same time and store in as7341 object
    if (!as7341.readAllChannels()) {
        Serial.println("Error reading all channels!");
        return;
    }

    // State logic
    switch (SYS_STATE) {
    case NOT_READY:
        // System is in an error state. This should never occur.
        Serial.println("System is in an error state. Please restart.");
        exit(1);
        break;
    case READY:
        // System is waiting for activation threshold
        if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) >= START_THRESHOLD) {
            SYS_STATE = RUNNING;
            // Print calibration message if in calibration mode, initialize reactionDuration
            if (CALIBRATION_MODE) {
                Serial.println("Starting calibration measurement.");
                reactionDuration = millis(); // For debugging and measurement purposes
            }
            // Turn on the pump
            if (PUMP_ACTIVE) { digitalWrite(PUMP_PIN, HIGH); }
            // Turn on the motor
            if (MOTOR_ACTIVE) { digitalWrite(MOTOR_PIN, HIGH); }
        }
        break;
    case RUNNING:
        // System is currently running the reaction
        // Print measurement values if requested
        if (VERBOSE_MODE) {
            printData();
        }
        // Check if the measurement has reached the end threshold
        if (as7341.getChannel(AS7341_CHANNEL_480nm_F3) <= END_THRESHOLD) {
            // Set system state to done, indicating reaction is complete
            SYS_STATE = DONE;
            // Print results if in calibration mode
            if (CALIBRATION_MODE) { printResults(); }
            // Turn off the motor
            if (MOTOR_ACTIVE) { digitalWrite(MOTOR_PIN, LOW); }
            // Turn off the pump
            if (PUMP_ACTIVE) { digitalWrite(PUMP_PIN, LOW); }
        }
        // Set LED color based off of measurement
        if (LED_ACTIVE) {
            // Map the 480nm channel to the LED color, with the threshold values as the range. 
            // The LED will be green at the start threshold and red at the end threshold.
            setLEDColor(
                map(as7341.getChannel(AS7341_CHANNEL_480nm_F3), START_THRESHOLD, END_THRESHOLD, 0, 255),    // Red
                map(as7341.getChannel(AS7341_CHANNEL_480nm_F3), START_THRESHOLD, END_THRESHOLD, 255, 0),    // Green
                0   // Blue
            );
        }
        break;
    case DONE:
        // System has completed the measurement
        Serial.println("Measurement complete.");
        // Set LEDs to show our school colors, orange and blue.
        if (LED_ACTIVE) {
            while (true) {
                colorWipe(Color::ORANGE, 50);
                delay(500);
                colorWipe(Color::BLUE, 50);
                delay(500);
            }
        }
        exit(0);
        break;
    }
}

// Helper Functions

/*
 * Print the measurement values for each channel.
 */
void printData() {
    // Print out the stored values for each channel
    Serial.print("480nm:");
    Serial.print(as7341.getChannel(AS7341_CHANNEL_480nm_F3));
    Serial.println("");
}

/*
 * Print the results of the calibration measurement.
 */
void printResults() {
    Serial.println("Measurement complete.");
    long finalTime = millis() - reactionDuration;
    Serial.print("Time: ");
    Serial.print(finalTime / 1000.0, 3);
    Serial.println(" seconds.");
}