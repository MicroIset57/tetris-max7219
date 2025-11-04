#include "Leds.h"

CRGB leds[NUM_LEDS];

void LedsInit()
{
    static bool inicializado = false;
    if (!inicializado)
    {
        FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
        // apago todo.
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        inicializado = true;
    }
}

void ShiftLed(CRGB col, uint8_t brillo /*255 default*/)
{
    static int led = 0;
    leds[led] = CRGB::Black;
    if (++led == NUM_LEDS)
        led = 0;

    leds[led] = col;
    if (brillo < 255)
        leds[led].nscale8(brillo);

    FastLED.show();
}

void NextLed(int desde, int hasta, CRGB col, bool dejarCola /*= true*/)
{
    static int led = desde;
    static bool sentido = true;
    if (!dejarCola)
        leds[led] = CRGB::Black;

    if (sentido)
    {
        if (++led == hasta)
            sentido = false;
    }
    else
    {
        if (--led == desde)
            sentido = true;
    }

    if (dejarCola)
        fadeToBlackBy(leds + desde, hasta - desde, 120);
    leds[led] = col;
    FastLED.show();
}

void ToggleLeds(CRGB col1, uint8_t brillo1, CRGB col2, uint8_t brillo2)
{
    static int c = 0;
    if (++c % 2 == 0)
        AllLeds(col1, brillo1);
    else
        AllLeds(col2, brillo2);
}

void AllLeds(CRGB col, uint8_t brillo)
{
    fill_solid(leds, NUM_LEDS, col);
    if (brillo < 255)
        fadeLightBy(leds, NUM_LEDS, brillo);
    FastLED.show();
}

void LedFlash(CRGB col)
{
    FastLED.setBrightness(255);
    fill_solid(leds, NUM_LEDS, col);
    FastLED.show();
    delay(50);
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
}

// "Knight Industries Two Thousand"
void LedsKitt(CRGB color)
{
    AllLeds(CRGB::Black);
    for (int i = 0; i < NUM_LEDS + 2; ++i)
    {
        if (i < NUM_LEDS)
            leds[i] = color;
        fadeToBlackBy(leds, NUM_LEDS, 70);
        FastLED.show();
        FastLED.delay(55);
    }
    for (int i = 0; i < NUM_LEDS * 2; ++i)
    {
        if (i < NUM_LEDS)
            leds[NUM_LEDS - 1 - i] = color;
        fadeToBlackBy(leds, NUM_LEDS, 70);
        FastLED.show();
        FastLED.delay(55);
    }
}

// Actualiza el arreglo de LEDs con un arcoiris que se desplaza hacia la derecha.
void updateRainbow()
{
    static uint8_t startHue = 0;
    // Usamos fill_rainbow para rellenar desde startHue.
    // Luego desplazamos hacia la derecha una posición para dar la sensación de movimiento.
    startHue += 4; // velocidad de cambio de color base
    fill_rainbow(leds, NUM_LEDS, startHue, 8);
    // Para que parezca que se mueve hacia la derecha, rotamos el array una posición a la right
    CRGB lastColor = leds[NUM_LEDS - 1];
    for (int i = NUM_LEDS - 1; i > 0; --i)
        leds[i] = leds[i - 1];
    leds[0] = lastColor;
    FastLED.show();
}

void SetLed(int ledNum, CRGB col)
{
    if (ledNum < 0 || ledNum >= NUM_LEDS)
        return;
    leds[ledNum] = col;
    FastLED.show();
}