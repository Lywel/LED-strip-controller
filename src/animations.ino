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

void arrayRotate(CRGB* buf, int n)
{
  if (!n)
    return;

  n %= STRIP_SIZE;

  if (n < 0)
    n = STRIP_SIZE + n;

  int count = 0;
  for (int start = 0; count < STRIP_SIZE; start++) {
      int current = start;
      CRGB prev = buf[start];
      do {
          int next = (current + n) % STRIP_SIZE;
          CRGB temp = buf[next];
          buf[next] = prev;
          prev = temp;
          current = next;
          count++;
      } while (start != current);
  }
}
