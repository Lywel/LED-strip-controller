#include "config.h"

byte ledGet(byte buf[], short id)
{
  return bitRead(buf[id / 8], id % 8);
}

void ledSet(byte buf[], short id, byte val)
{
  bitWrite(buf[id / 8], id % 8, val);
}



/*
 * The setup function configuring the hardware.
 * This is called once on boot and on reset.
 */

void setup()
{
  // Obscure 'security' delay...
  // It should be 2000 but I don't have time for that shit
  delay(500);

  pinMode(POTAR_PIN, INPUT);
  pinMode(BTN1_PIN, INPUT);
  pinMode(BTN2_PIN, INPUT);

  /*
   * FastLED Leds initilization
   * led type: WS2812B
   * data pin: STRIP_PIN
   * colors order: GRB
   * CRGB array: leds[STRIP_SIZE]
   */
  FastLED.addLeds<WS2812B, STRIP_PIN, GRB>(leds, STRIP_SIZE);
  FastLED.setMaxPowerInVoltsAndMilliamps(MAX_VOLTS, MAX_MAMPS);
  set_max_power_indicator_LED(13);

  FastLED.setBrightness(20);

  //checkMemory();
  FastLED.showColor(CRGB::White);
}

void loop()
{
  bool btn1 = debounceButton(BTN1_PIN);
  bool btn2 = debounceButton(BTN2_PIN);

  if (btn2)
    configMode();

  if (btn1)
  {
    short potarRead = analogRead(POTAR_PIN);
    FastLED.setBrightness(map(potarRead, 0, 1023, 0, 255));
    //FastLED.show();
    FastLED.showColor(CRGB::White);
  }
}

void ledcpy(CRGB *dst, CRGB *src)
{
  short i = STRIP_SIZE;
  while (i--) *dst++ = *src++;
}

void configMode()
{
  FastLED.clear(true);
  ledcpy(leds_temp, leds);

  FastLED.clear(true);
  bool btn1 = false;
  bool btn2 = false;

  while (!btn2)
  {
    short potarRead = analogRead(POTAR_PIN);
    short currentLED = map(potarRead, 0, 1023, 0, STRIP_SIZE - 1);

    ledcpy(leds, leds_temp);
    leds[currentLED] = CRGB::Red;
    FastLED.show();

    btn1 = debounceButton(BTN1_PIN);
    btn2 = debounceButton(BTN2_PIN);
    if (btn1)
    {
      if (leds_temp[currentLED])
        leds_temp[currentLED] = CRGB::Black;
      else
        leds_temp[currentLED] = CRGB::White;
    }
  }

  ledcpy(leds, leds_temp);
}
