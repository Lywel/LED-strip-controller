#pragma once

#include "FastLED.h"

// Pin config
#define POTAR_PIN A0
#define BTN1_PIN 5
#define BTN2_PIN 6
#define STRIP_PIN 3


// LED groups memory config

// Nuber of leds to use
#define STRIP_SIZE 100
// Maximum number of led groups
#define MAX_GROUPS 12

// Set the capacity of the power supply
#define MAX_VOLTS 5
#define MAX_MAMPS 2000

// leds state mapped on bits for optimisation
byte led_set[STRIP_SIZE / 8 + 1];

// The real array of leds. One item for each led in the strip.
CRGB leds[STRIP_SIZE];
CRGB leds_temp[STRIP_SIZE];

byte lastButtonState[13];
byte buttonState[13];
unsigned long lastDebounceTime[13];
unsigned debouceDelay = 50;
