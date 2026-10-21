#include <Arduino.h>
#include "Controles.h"

// ========================================
// Control de Pulsadores por interrupcion
// ========================================
// Cada pulsador dispara una interrupcion en flanco descendente (INPUT_PULLUP,
// activo en LOW). El debounce se hace por tiempo dentro de la ISR y cada
// pulsacion detectada se acumula en un buffer, para no perder pulsaciones
// rapidas mientras el loop esta ocupado (ej: dibujando el frame).

#define NUM_BOTONES 4
#define BUFFER_PULSACIONES 4   // pulsaciones en cola por boton
#define DEBOUNCE_US 150000UL   // 150 ms, igual que el debounce anterior

static const int pines[NUM_BOTONES] = {BTN_IZQUIERDA, BTN_DERECHA, BTN_BAJAR, BTN_ROTAR};

static volatile uint8_t pulsacionesPendientes[NUM_BOTONES] = {0};
static volatile unsigned long ultimaInterrupcion[NUM_BOTONES] = {0};
static portMUX_TYPE botonMux = portMUX_INITIALIZER_UNLOCKED;

static void IRAM_ATTR isrBoton(void *arg)
{
    int idx = (int)(intptr_t)arg;
    unsigned long ahora = micros();

    portENTER_CRITICAL_ISR(&botonMux);
    if (ahora - ultimaInterrupcion[idx] > DEBOUNCE_US)
    {
        ultimaInterrupcion[idx] = ahora;
        if (pulsacionesPendientes[idx] < BUFFER_PULSACIONES)
        {
            pulsacionesPendientes[idx] = pulsacionesPendientes[idx] + 1;
        }
    }
    portEXIT_CRITICAL_ISR(&botonMux);
}

void setupControles()
{
    for (int i = 0; i < NUM_BOTONES; i++)
    {
        pinMode(pines[i], INPUT_PULLUP);
        attachInterruptArg(digitalPinToInterrupt(pines[i]), isrBoton, (void *)(intptr_t)i, FALLING);
    }
}

bool btnPressed(int pin)
{
    int idx = -1;
    for (int i = 0; i < NUM_BOTONES; i++)
    {
        if (pines[i] == pin)
        {
            idx = i;
            break;
        }
    }
    if (idx < 0)
    {
        return false;
    }

    bool hayPulsacion = false;
    portENTER_CRITICAL(&botonMux);
    if (pulsacionesPendientes[idx] > 0)
    {
        pulsacionesPendientes[idx] = pulsacionesPendientes[idx] - 1;
        hayPulsacion = true;
    }
    portEXIT_CRITICAL(&botonMux);

    return hayPulsacion;
}
