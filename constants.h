#ifndef CONSTANTS_H
#define CONSTANTS_H

// Digital Pins
#define LED_PIN 3
#define MOTOR_PIN 11
#define PUMP_PIN 10

// LED Count
#define LED_COUNT 14 

/*
 * MEASUREMENT THRESHOLD VALUES:
 * For the Luminol reaction, the threshold values are:
 * Start: 1000, End: 100
 * For testing purposes, the threshold values are:
 * Start: 500, End: 50
 */
uint16_t START_THRESHOLD = 300;
uint16_t END_THRESHOLD = 50;

#endif // CONSTANTS_H