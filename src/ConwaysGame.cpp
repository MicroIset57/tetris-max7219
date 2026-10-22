/*
  Juego de la Vida (Conway)
  --------------------------------------------
  Evoluciona solo, sin jugador, sobre toda la matriz (14x25). Arranca
  con una semilla al azar y corre hasta que se presiona un boton. Si
  una generacion muere o deja de cambiar (estado estable), se vuelve a
  sembrar al azar para que siempre haya algo en pantalla.

  Botones:
  - Izquierda / Derecha: resetean el juego (nueva semilla al azar).
  - Arriba / Abajo: abortan y siguen con la presentacion/menu normal.

  Reglas clasicas B3/S23, sin wrap-around: los bordes de la matriz son
  el borde del universo (los vecinos fuera de rango no cuentan).

  Javier Rambaldo
  Octubre 2026.
*/

#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"

static const int DENSIDAD_SEMILLA = 28;         // % de celdas vivas al sembrar
static const unsigned long PERIODO_GENERACION_MS = 180; // tiempo entre generaciones
static const unsigned long PAUSA_FIN_MS = 800;  // pausa al llegar a un estado muerto/estable

static uint8_t celdas[MATRIZ_ALTO][MATRIZ_ANCHO];
static uint8_t siguiente[MATRIZ_ALTO][MATRIZ_ANCHO];

// ========================================
// Semilla al azar: cada celda vive con probabilidad DENSIDAD_SEMILLA%.
// ========================================
static void sembrarAlAzar()
{
    for (int y = 0; y < MATRIZ_ALTO; y++)
        for (int x = 0; x < MATRIZ_ANCHO; x++)
            celdas[y][x] = random(0, 100) < DENSIDAD_SEMILLA;
}

// ========================================
// Vecinos vivos (8 direcciones), sin wrap-around: lo que cae fuera de
// la matriz simplemente no cuenta.
// ========================================
static int contarVecinos(int x, int y)
{
    int total = 0;
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
                continue;
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < MATRIZ_ANCHO && ny >= 0 && ny < MATRIZ_ALTO)
                total += celdas[ny][nx];
        }
    }
    return total;
}

static int contarVivas()
{
    int total = 0;
    for (int y = 0; y < MATRIZ_ALTO; y++)
        for (int x = 0; x < MATRIZ_ANCHO; x++)
            total += celdas[y][x];
    return total;
}

// ========================================
// Calcula la proxima generacion (reglas B3/S23). Devuelve true si
// hubo algun cambio respecto de la generacion anterior.
// ========================================
static bool siguienteGeneracion()
{
    bool huboCambios = false;

    for (int y = 0; y < MATRIZ_ALTO; y++)
    {
        for (int x = 0; x < MATRIZ_ANCHO; x++)
        {
            int vecinos = contarVecinos(x, y);
            bool viva = celdas[y][x];
            bool vivaSiguiente = viva ? (vecinos == 2 || vecinos == 3) : (vecinos == 3);

            siguiente[y][x] = vivaSiguiente;
            if (vivaSiguiente != viva)
                huboCambios = true;
        }
    }

    memcpy(celdas, siguiente, sizeof(celdas));
    return huboCambios;
}

static void dibujarCeldas()
{
    matrix.clearBuffer();
    for (int y = 0; y < MATRIZ_ALTO; y++)
        for (int x = 0; x < MATRIZ_ANCHO; x++)
            if (celdas[y][x])
                matrix.set(x, y);
    matrix.render();
}

// izquierda o derecha: reiniciar con una semilla nueva.
static bool hayPulsacionReset()
{
    return btnPressed(BTN_IZQUIERDA) || btnPressed(BTN_DERECHA);
}

// arriba o abajo: abortar y seguir con la presentacion/menu normal.
static bool hayPulsacionAbortar()
{
    return btnPressed(BTN_ARRIBA) || btnPressed(BTN_BAJAR);
}

// ========================================
// Corre la simulacion hasta que se presione Arriba/Abajo (Izquierda/
// Derecha solo la reinician con una semilla nueva).
// ========================================
void jugarConway()
{
    Serial.println("Conway's Game of Life (solo se ve despues de un reset)...");

    randomSeed(esp_random());
    sembrarAlAzar();

    unsigned long tiempoGeneracion = millis();

    while (!hayPulsacionAbortar())
    {
        if (hayPulsacionReset())
        {
            sembrarAlAzar();
            tiempoGeneracion = millis();
        }

        dibujarCeldas();

        if (millis() - tiempoGeneracion > PERIODO_GENERACION_MS)
        {
            tiempoGeneracion = millis();
            bool huboCambios = siguienteGeneracion();

            if (!huboCambios || contarVivas() == 0)
            {
                delay(PAUSA_FIN_MS); // se deja ver el estado final antes de volver a sembrar
                sembrarAlAzar();
            }
        }

        delay(10);
    }

    matrix.clearBuffer();
    matrix.render();
}
