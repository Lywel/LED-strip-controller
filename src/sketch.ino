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
  FastLED.setCorrection(TypicalSMD5050);

  // Load settings from EEPROM or initialize it
  initConfig();

  // Load the first sequence to display
  reloadSeq();
}


uint32_t lastRotation = 0;
uint32_t rotationInterval = 0;

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
  // Handle brightness
  uint16_t potarRead = analogRead(POTAR_PIN);
  if (abs(potarRead - lastPotarRead) > 2)
  {
    lastPotarRead = potarRead;
    FastLED.show(constrain(
      map(potarRead, 0, 1023, 0, 255),
      0, 255));
  }

  // Handle buttons
  bool btn1 = debounceButton(BTN1_PIN);
  bool btn2 = debounceButton(BTN2_PIN);


  // loop through sequences
  if (btn1)
  {
    ++(config.currentSeq) %= MAX_SEQ;
    reloadSeq();
  }

  // Open the sequence editor
  if (btn2)
  {
    // save state and copy the leds to a live buffer to save EEPROM io
    int8_t seq = config.currentSeq;

    sequenceManager();

    // restore saved state (not EEPROM io)
    config.currentSeq = seq;
    reloadSeq();
  }

  // Sequence rotation
  uint16_t potarRead = analogRead(POTAR_PIN);
  uint32_t now = millis();


  if (potarRead > 614)
    rotationInterval = 3000 / (potarRead - 614);
  else if (potarRead < 410)
    rotationInterval = 3000 / (410 - potarRead);
  else
    rotationInterval = 10;

  if (now - lastRotation >= rotationInterval)
  {
    lastRotation = now;
    arrayRotate(leds, potarRead < 410 ? -1 : (potarRead > 614 ? 1 : 0));
    FastLED.show();
  }
}



/*
 * Sequence Manager
 * - Select a sequence to edit
 * - Edit it
 * - Save it
 */
void sequenceManager()
{
  // Select the sequence to edit
  config.currentSeq = sequenceSelector();
  // If user canceled return to display mode
  if (config.currentSeq < 0)
    return;

  FastLED.clear(true);

  // Load the sequence
  reloadSeq();
  // Save the sequence to a buffer
  memcpy(temp2, leds, STRIP_SIZE * sizeof(CRGB));

  // If this is a new sequence, set a default brightness (otherwise it will
  // be saved to 0 and then considered as a free sequence)
  if (seqBrightness == 0)
    seqBrightness = MENU_BRIGHTNESS;

  uint8_t currentLED = 0;

  while (true)
  {
    // Handle inputs
    bool btn1 = debounceButton(BTN1_PIN);
    bool btn2 = debounceButton(BTN2_PIN);
    uint16_t potarRead = analogRead(POTAR_PIN);

    // Clear the old cursor
    leds[currentLED] = temp2[currentLED];

    currentLED = constrain(
      map(potarRead, 0, 1023, 0, STRIP_SIZE),
      0, STRIP_SIZE-1);

    // Set the new cursor
    leds[currentLED] = blinkColor(temp2[currentLED], CRGB::Blue);

    // Display the strip + blinking cursor
    FastLED.show(MENU_BRIGHTNESS);

    if (btn2)
    {
      Serial.println("Click 2");
      if (sequenceEditorMenu() == -1)
        break;
    }

    // Set / unset currentLED
    if (btn1)
    {
      Serial.println("Click 1");
      temp2[currentLED] = temp2[currentLED] ? CRGB::Black : CRGB::White;
    }
  }
}



/* Sequence editor menu
 * - Save edits
 * - Discrad edits
 * - Select the brightness
 * - TODO: Back to edition
 * - TODO: Pick color
 */
int8_t sequenceEditorMenu()
{
  FastLED.clear(true);

  while (true)
  {
    // Handle inputs
    bool btn1 = debounceButton(BTN1_PIN);
    uint16_t potarRead = analogRead(POTAR_PIN);
    uint8_t ledId = constrain(map(potarRead, 0, 1023, 0, 3), 0, 2);


    // Reset the menu
    leds[0] = CRGB::Red;
    leds[1] = CRGB::Green;
    leds[2] = brightnessAnimation();
    leds[3] = rainbowAnimation();

    // Set the cursor
    leds[ledId] = blinkColor(leds[ledId], CRGB::Blue);

    // Display the menu + blinking cursor
    FastLED.show(MENU_BRIGHTNESS);

    // On click
    if (btn1)
    {
      // Quit without saving
      if (ledId == 0)
        break;
      else if (ledId == 1)
      {
        // TODO: use the selected brightness
        saveSequence();
        break;
      }
      else if (ledId == 2)
      {
        seqBrightness = brightnessSelector();
        return 0;
      }
    }
  }

  return -1;
}



uint8_t brightnessSelector()
{
  // Show thw current sequence
  memcpy(leds, temp2, STRIP_SIZE * sizeof(CRGB));

  for (uint8_t b = MENU_BRIGHTNESS; b > 0; --b)
  {
    FastLED.show(b);
    delay(1);
  }

  uint16_t potarRead = analogRead(POTAR_PIN);
  uint8_t selectedBrightness = constrain(map(potarRead, 0, 1023, 0, 255), 0, 255);

  for (uint8_t b = 0; b < selectedBrightness; ++b)
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



int8_t sequenceSelector()
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
