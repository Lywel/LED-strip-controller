#include "config.h"

CHSV rainbowAnimation()
{
  return CHSV(millis() / 16 % 255, 255, 128);
}

CRGB brightnessAnimation()
{
  uint8_t c = abs((int64_t)(millis() / 16 % 255) - 127) * 2;
  return CRGB(c, c, c);
}

CRGB blinkColor(CRGB on, CRGB off)
{
  uint32_t currentMs = millis();
  if (currentMs - lastBlinkMs >= BLINK_INTER_MS)
  {
    lastBlinkMs = currentMs;
    blinkState = !blinkState;
  }
  return blinkState ? on : off;
}
