#include "config.h"

bool debounceButton(byte btn)
{
  int reading = digitalRead(btn);

  if (reading != lastButtonState[btn])
    lastDebounceTime[btn] = millis();

  if ((millis() - lastDebounceTime[btn]) > debouceDelay)
  {
    if (reading != buttonState[btn])
    {
      buttonState[btn] = reading;
      return buttonState[btn] == HIGH;
    }
  }

  lastButtonState[btn] = reading;
  return false;
}
