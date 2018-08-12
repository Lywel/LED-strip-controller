#include "config.h"
#include <EEPROM.h>

/*
 * The setup function configuring the hardware.
 * This is called once on boot and on reset.
 */

void setup()
{
  Serial.begin(9600);

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
  set_max_power_in_volts_and_milliamps(MAX_VOLTS, MAX_MAMPS);
  set_max_power_indicator_LED(13);
  FastLED.setBrightness(20);

  // Load settings from EEPROM or initialize it
  initConfig();

  // Load the first sequence to display
  reloadSeq();
}

/*
 * Main Loop
 * This is the 'display' mode
 * It displays the stored sequences
 * - Potard dims brightness
 * - BTN1 goes to the next sequences (loops to the first at the end)
 * - BTN2 leads to the configuration mode
 */

void loop()
{
  // Handle buttons
  bool btn1 = debounceButton(BTN1_PIN);
  bool btn2 = debounceButton(BTN2_PIN);


  if (btn1)
  {
    ++(config.currentSeq) %= MAX_SEQ;
    reloadSeq();
  }

  if (btn2)
  {
    // save state and copy the leds to a live buffer to save EEPROM io
    int8_t seq = config.currentSeq;
    uint8_t brightness = FastLED.getBrightness();
    memcpy(temp1, leds, STRIP_SIZE * sizeof(CRGB));

    configMode();

    // restore saved state (not EEPROM io)
    memcpy(leds, temp1, STRIP_SIZE * sizeof(CRGB));
    config.currentSeq = seq;

    FastLED.show(brightness);
  }
}


void configMode()
{
  // Select the sequence to edit
  config.currentSeq = selectSeq();
  // If user canceled return to display mode
  if (config.currentSeq < 0)
    return;

  FastLED.clear(true);

  // Load the sequence
  reloadSeq();
  memcpy(temp2, leds, STRIP_SIZE * sizeof(CRGB));

  while (true)
  {
    // Handle inputs
    bool btn1 = debounceButton(BTN1_PIN);
    bool btn2 = debounceButton(BTN2_PIN);
    uint16_t potarRead = analogRead(POTAR_PIN);
    uint8_t currentLED =
      constrain(map(potarRead, 0, 1023, 0, STRIP_SIZE), 0, STRIP_SIZE-1);

    // Reset the strip
    memcpy(leds, temp2, STRIP_SIZE * sizeof(CRGB));

    // Set the cursor
    leds[currentLED] = blinkColor(temp2[currentLED], CRGB::Blue);

    // Display the strip + blinking cursor
    FastLED.show(MENU_BRIGHTNESS);

    // Set/unset currentLED
    if (btn1)
      temp2[currentLED] = temp2[currentLED] ? CRGB::Black : CRGB::White;

    if (btn2)
      break;
  }

  uint8_t brightness = selectBrightness();

  saveSeq(brightness);
}

uint8_t selectBrightness()
{
  for (uint8_t b = MENU_BRIGHTNESS; b < 127; ++b)
  {
    FastLED.show(b);
    delay(1);
  }

  for (uint8_t b = 127; b > 0; --b)
  {
    FastLED.show(b);
    delay(1);
  }

  uint16_t potarRead = analogRead(POTAR_PIN);
  uint8_t currentBrightness = constrain(map(potarRead, 0, 1023, 0, 255), 0, 255);

  for (uint8_t b = 0; b < currentBrightness; ++b)
  {
    FastLED.show(b);
    delay(1);
  }


  while (true)
  {
    // Handle inputs
    bool btn1 = debounceButton(BTN1_PIN);
    uint16_t potarRead = analogRead(POTAR_PIN);
    FastLED.show(constrain(
      map(potarRead, 0, 1023, 0, 255),
      0, 255));

    if (btn1)
      return constrain(map(potarRead, 0, 1023, 0, 255), 0, 255);
  }
}

int8_t selectSeq()
{
  FastLED.clear(true);

  int32_t usedSeq = 0;

  // Menu
  // Sequences (free one are unlit)
  for (int8_t i = 0; i < MAX_SEQ; ++i)
  {
    uint16_t brightnessIndex =
      sizeof(EEPROMConfig)
      + (i+1) * STRIP_SIZE * sizeof(CRGB)
      + i * sizeof(uint8_t);
    bitWrite(usedSeq, i, EEPROM[brightnessIndex] != 0);
    leds[i] = bitRead(usedSeq, i) ? CRGB::White : CRGB::Black;
  }

  // OK / KO
  leds[MAX_SEQ] = CRGB::Red;

  int8_t selectedSeq = -1;
  CRGB cursorColor = CRGB::Blue;

  while (true)
  {
    // Handle inputs
    bool btn1 = debounceButton(BTN1_PIN);
    uint16_t potarRead = analogRead(POTAR_PIN);
    int8_t ledId = constrain(
        map(potarRead, 0, 1023, 0, MAX_SEQ + 2),
        0, MAX_SEQ + (selectedSeq >= 0));

    // Menu
    // Sequences (free one are unlit)
    for (int8_t i = 0; i < MAX_SEQ; ++i)
    {
      if (i == selectedSeq)
        leds[i] = cursorColor;
      else
        leds[i] = bitRead(usedSeq, i) ? CRGB::White : CRGB::Black;
    }

    // OK / KO
    leds[MAX_SEQ] = CRGB::Red;

    if (selectedSeq >= 0)
      leds[MAX_SEQ + 1] = CRGB::Green;

    // Set the cursor
    if (!memcmp(leds[ledId].raw, cursorColor.raw, 3))
      leds[ledId] = blinkColor(CRGB::Black, CRGB::Blue);
    else
      leds[ledId] = blinkColor(leds[ledId], CRGB::Blue);


    // Display the menu + blinking cursor
    FastLED.show(MENU_BRIGHTNESS);

    if (btn1)
    {
      Serial.println(selectedSeq);
      if (ledId < MAX_SEQ)
        selectedSeq = ledId;
      else if (ledId == MAX_SEQ)
        return -1;
      else
        break;
    }
  }

  return selectedSeq;
}

void saveSeq(uint8_t brightness)
{
  FastLED.clear(true);

  // Menu KO / OK
  leds[0] = CRGB::Red;
  leds[1] = CRGB::Green;

  uint8_t ok;

  while (true)
  {
    // Handle inputs
    bool btn1 = debounceButton(BTN1_PIN);
    uint16_t potarRead = analogRead(POTAR_PIN);
    ok =
      constrain(map(potarRead, 0, 1023, 0, 2), 0, 1);

    // Reset the menu
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Green;

    // Set the cursor
    leds[ok] = blinkColor(leds[ok], CRGB::Blue);

    // Display the menu + blinking cursor
    FastLED.show(MENU_BRIGHTNESS);

    if (btn1)
      break;
  }

  if (ok)
  {
    for (uint8_t i = 0; i < STRIP_SIZE; ++i)
      writeSeqLED(i, temp2[i]);

    uint16_t brightnessIndex =
      sizeof(EEPROMConfig)
      + (config.currentSeq+1) * STRIP_SIZE * sizeof(CRGB)
      + config.currentSeq * sizeof(uint8_t);
    EEPROM[brightnessIndex] = brightness;

    EEPROM.put(0, config);
  }
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
