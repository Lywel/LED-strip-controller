#include "config.h"

bool debounceButton(int8_t btn)
{
  uint32_t reading = digitalRead(btn);

  if (reading != lastButtonState[btn%2])
    lastDebounceTime[btn%2] = millis();

  if ((millis() - lastDebounceTime[btn%2]) > debouceDelay)
  {
    if (reading != buttonState[btn%2])
    {
      buttonState[btn%2] = reading;
      return buttonState[btn%2] == HIGH;
    }
  }

  lastButtonState[btn%2] = reading;
  return false;
}
