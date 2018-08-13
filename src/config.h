#pragma once

#include "FastLED.h"

// Pin config
#define POTAR_PIN A1
#define BTN1_PIN A0
#define BTN2_PIN A2
#define STRIP_PIN 3


// LED groups memory config

// Nuber of leds to use
#define STRIP_SIZE 75
// Maximum number of led groups
#define MAX_GROUPS 12

// Set the capacity of the power supply
#define MAX_VOLTS 5
#define MAX_MAMPS 2000

// Cursor blink
#define BLINK_INTER_MS 200
uint32_t lastBlinkMs = 0;
int8_t blinkState = LOW;

#define MENU_BRIGHTNESS 30

// The real array of leds. One item for each led in the strip.
CRGB leds[STRIP_SIZE];
CRGB temp2[STRIP_SIZE];

// Button debounce system
uint8_t debouceDelay = 50;
int8_t lastButtonState[2];
int8_t buttonState[2];
uint32_t lastDebounceTime[2];

uint16_t lastPotarRead = 0;

uint8_t seqBrightness;

/*
 * System settings
 * Read from EEPROM if
 * - it looks valid
 * - the length of the sequence matches the current STRIP_SIZE
 */

struct EEPROMConfig {
  uint8_t stripSize = STRIP_SIZE;
  int8_t currentSeq = 0;
} config;

#define EEPROM_SIZE 1024
#define MAX_SEQ ((EEPROM_SIZE - sizeof(EEPROMConfig)) / ((STRIP_SIZE + 1) * sizeof(CRGB)))
