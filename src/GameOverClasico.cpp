/*
  Cartel clasico de "GAME OVER".
  ------------------------------
  Se muestra cuando se pierde 3 veces seguidas (en cualquiera de los
  juegos), y despues deja pendiente volver al menu principal.

  Javier Rambaldo
  Octubre 2026.
*/

#include <Arduino.h>
#include "LedMatrix.h"
#include "Fuente5x7.h"
#include "GameOverClasico.h"

// clang-format off
static const uint8_t gameOverCols[] = {
    LETRA_G, LETRA_A, LETRA_M, LETRA_E, ESPACIO,                   // GAME
    LETRA_O, LETRA_V, LETRA_E, LETRA_R, ESPACIO, ESPACIO, ESPACIO,  // OVER
};
// clang-format on
static const int GAMEOVER_LEN = sizeof(gameOverCols) / sizeof(gameOverCols[0]);

static int derrotasSeguidas = 0;
static bool pedidoDeMenu = false;

// ========================================
// "GAME OVER" recorriendo la pantalla de derecha a izquierda.
// ========================================
static void mostrarGameOverClasico()
{
    const int filaSuperior = 18; // misma altura que usa el menu
    const unsigned long duracionMs = 4000;
    unsigned long inicio = millis();
    int offset = 0;

    while (millis() - inicio < duracionMs)
    {
        matrix.clearBuffer();
        for (int x = 0; x < MATRIZ_ANCHO; x++)
        {
            uint8_t col = gameOverCols[(offset + x) % GAMEOVER_LEN];
            for (int fila = 0; fila < 7; fila++)
                if (col & (1 << fila))
                    matrix.set(x, filaSuperior - fila);
        }
        matrix.render();
        offset = (offset + 1) % GAMEOVER_LEN;
        delay(120);
    }

    matrix.clearBuffer();
    matrix.render();
}

void registrarDerrota()
{
    derrotasSeguidas++;
    if (derrotasSeguidas >= 3)
    {
        derrotasSeguidas = 0;
        mostrarGameOverClasico();
        pedidoDeMenu = true;
    }
}

bool hayPedidoDeMenu()
{
    if (pedidoDeMenu)
    {
        pedidoDeMenu = false;
        return true;
    }
    return false;
}
