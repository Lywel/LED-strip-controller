#include "config.h"
#include <EEPROM.h>

// If we use bits to store lights
byte ledGet(byte buf[], short id)
{
  return bitRead(buf[id / 8], id % 8);
}

void ledSet(byte buf[], short id, byte val)
{
  bitWrite(buf[id / 8], id % 8, val);
}


void initConfig()
{
  EEPROM.get(0, config);

  if (config.stripSize != STRIP_SIZE)
  {
    for (int8_t i = 0; i < MAX_SEQ; ++i)
    {
      uint16_t brightnessIndex =
        sizeof(EEPROMConfig)
        + (i+1) * STRIP_SIZE * sizeof(CRGB)
        + i * sizeof(uint8_t);
      EEPROM[brightnessIndex] = 0;
    }

    EEPROMConfig cleanConfig;
    config = cleanConfig;
    EEPROM.put(0, config);
  }
}

CRGB readSeqLED(uint8_t LEDIndex)
{
  CRGB res;
  EEPROM.get(sizeof(EEPROMConfig)                     // Skip the config
    + config.currentSeq * STRIP_SIZE * sizeof(CRGB)   // Skip n strips
    + config.currentSeq * sizeof(uint8_t)             // Skip n brightness bytes
    + LEDIndex * sizeof(CRGB)                         // Get LEDIndex
    , res);
  return res;
}

void writeSeqLED(uint8_t LEDIndex, CRGB color)
{
  EEPROM.put(sizeof(EEPROMConfig)                     // Skip the config
    + config.currentSeq * STRIP_SIZE * sizeof(CRGB)   // Skip n strips
    + config.currentSeq * sizeof(uint8_t)             // Skip n brightness bytes
    + LEDIndex * sizeof(CRGB)                         // Get LEDIndex
    , color);
}

void reloadSeq()
{
  Serial.print("EEPROM: reloading sequence ");
  Serial.println(config.currentSeq);

  uint16_t brightnessIndex =
    sizeof(EEPROMConfig)
    + (config.currentSeq+1) * STRIP_SIZE * sizeof(CRGB)
    + config.currentSeq * sizeof(uint8_t);

  uint8_t brightness = EEPROM[brightnessIndex];

  if (brightness > 0)
  {
    for (uint8_t i = 0; i < STRIP_SIZE; ++i)
    {
      leds[i] = readSeqLED(i);
      Serial.print(!!leds[i]);
    }
  }

  Serial.print("\r\nBrightness: ");
  Serial.println(brightness);

  FastLED.show(brightness);
}
