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

// Cursor blink
#define BLINK_INTER_MS 200
uint32_t lastBlinkMs = 0;
int8_t blinkState = LOW;

// leds state mapped on bits for optimisation
//int8_t led_set[STRIP_SIZE / 8 + 1];

// The real array of leds. One item for each led in the strip.
CRGB leds[STRIP_SIZE];
CRGB temp1[STRIP_SIZE];
CRGB temp2[STRIP_SIZE];

// Button debounce system
uint8_t debouceDelay = 50;
int8_t lastButtonState[6];
int8_t buttonState[6];
uint32_t lastDebounceTime[13];

uint16_t lastPotarRead = 0;


/*
 * System settings
 * Read from EEPROM if
 * - it looks valid
 * - the length of the sequence matches the current STRIP_SIZE
 */

struct EEPROMConfig {
  uint8_t stripSize = STRIP_SIZE;
  int8_t currentSeq = 0;
  int16_t usedSeq = 0; // bitset (16)
} config;

const int16_t MAX_SEQ = 1020 / (STRIP_SIZE * sizeof(CRGB));
