#pragma once
#include <FastLED.h>

#define NUM_LEDS 14
#define LED_PIN 12

// donde comienza el led del ISET 57 y donde termina:
// 5 primeros leds son dias L-M-M-J-V
#define ISET57_BEGIN 5
#define ISET57_END (NUM_LEDS - 1)

void LedsInit();
void NextLed(int desde, int hasta, CRGB col, bool dejarCola = true);
void ShiftLed(CRGB col, uint8_t brillo = 255);
void AllLeds(CRGB col, uint8_t brillo = 255);
void LedFlash(CRGB col);
void LedsKitt(CRGB color);
void ToggleLeds(CRGB col1, uint8_t brillo1, CRGB col2, uint8_t brillo2);
void updateRainbow();
void SetLed(int ledNum, CRGB col);
