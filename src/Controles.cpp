#include <Arduino.h>
#include "Controles.h"

void setupControles()
{
    // Configurar pulsadores
    pinMode(BTN_IZQUIERDA, INPUT_PULLUP);
    pinMode(BTN_DERECHA, INPUT_PULLUP);
    pinMode(BTN_BAJAR, INPUT_PULLUP);
    pinMode(BTN_ROTAR, INPUT_PULLUP);
}

// ========================================
// Control de Pulsadores
// ========================================
bool btnPressed(int pin)
{
    static unsigned long lastPress[4] = {0};
    static int pins[4] = {BTN_IZQUIERDA, BTN_DERECHA, BTN_BAJAR, BTN_ROTAR};

    int idx = 0;
    for (int i = 0; i < 4; i++)
    {
        if (pins[i] == pin)
        {
            idx = i;
            break;
        }
    }

    if (digitalRead(pin) == LOW && (millis() - lastPress[idx] > 150))
    {
        lastPress[idx] = millis();
        return true;
    }
    return false;
}
