#ifndef HARDWARE_H
#define HARDWARE_H

#include <Adafruit_AS7341.h>
#include <Adafruit_NeoPixel.h>
#include "constants.h"

// Spectrometer sensor
Adafruit_AS7341 as7341;

// NeoPixel LED strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#endif // HARDWARE_H