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
    EEPROM.put(0, config);
}

CRGB readSeqLED(uint8_t LEDIndex)
{
  CRGB res;
  EEPROM.get(sizeof(EEPROMConfig)
    + (config.currentSeq * STRIP_SIZE + LEDIndex) * sizeof(CRGB), res);
  return res;
}

void writeSeqLED(uint8_t LEDIndex, CRGB color)
{
  EEPROM.put(sizeof(EEPROMConfig)
    + (config.currentSeq * STRIP_SIZE + LEDIndex) * sizeof(CRGB), color);
}

void reloadSeq()
{
  Serial.print("EEPROM: reloading sequence ");
  Serial.println(config.currentSeq);

  if (bitRead(config.usedSeq, config.currentSeq))
  {
    for (uint8_t i = 0; i < STRIP_SIZE; ++i)
    {
      leds[i] = readSeqLED(i);
      Serial.print(!!leds[i]);
    }
    Serial.println();
  }
  else
  {
    FastLED.clear(true);
    Serial.println("Sequence is not set");
  }

  FastLED.show();
}
