#include "config.h"

bool debounceButton(int8_t btn)
{
  uint32_t reading = digitalRead(btn);

  if (reading != lastButtonState[btn == A2])
    lastDebounceTime[btn == A2] = millis();

  if ((millis() - lastDebounceTime[btn == A2]) > debouceDelay)
  {
    if (reading != buttonState[btn == A2])
    {
      buttonState[btn == A2] = reading;
      return buttonState[btn == A2] == HIGH;
    }
  }

  lastButtonState[btn == A2] = reading;
  return false;
}
