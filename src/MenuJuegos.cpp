/*
  Menu inicial para elegir juego: "DINO", "TETRIS", "VIVORA" y
  "ARKANOID" se desplazan sin parar de derecha a izquierda; el primer
  boton que se presiona selecciona el titulo que esta en pantalla en
  ese momento.

  Javier Rambaldo
  Octubre 2026.
*/

#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"
#include "EfectoOla.h"
#include "Fuente5x7.h"
#include "MenuJuegos.h"

// ========================================
// Indice (columna) donde arranca cada palabra dentro de menuCols[],
// en el mismo orden en que aparecen (ciclico). Cada zona incluye la
// brecha que la sigue, hasta que arranca la letra de la proxima.
static const int ZONA_DINO = 0;
static const int ZONA_TETRIS = 27;    // 4 letras DINO (4*6=24) + brecha (3)
static const int ZONA_VIVORA = 66;    // + 6 letras TETRIS (6*6=36) + brecha (3)
static const int ZONA_ARKANOID = 105; // + 6 letras VIVORA (6*6=36) + brecha (3)
static const int MENU_LEN = 156;      // + 8 letras ARKANOID (8*6=48) + brecha (3)

static const uint8_t menuCols[MENU_LEN] = {
    LETRA_D, SEP, LETRA_I, SEP, LETRA_N, SEP, LETRA_O, SEP, ESPACIO, // DINO
    LETRA_T, SEP, LETRA_E, SEP, LETRA_T, SEP, LETRA_R, SEP,
    LETRA_I, SEP, LETRA_S, SEP, ESPACIO, // TETRIS
    LETRA_V, SEP, LETRA_I, SEP, LETRA_V, SEP, LETRA_O, SEP,
    LETRA_R, SEP, LETRA_A, SEP, ESPACIO, // VIVORA
    LETRA_A, SEP, LETRA_R, SEP, LETRA_K, SEP, LETRA_A, SEP,
    LETRA_N, SEP, LETRA_O, SEP, LETRA_I, SEP, LETRA_D, SEP, ESPACIO, // ARKANOID
};
// clang-format on

// ========================================
// A que juego corresponde la columna que en este momento esta en el
// borde izquierdo de la pantalla (x=0).
// ========================================
static int juegoEnOffset(int offset)
{
    if (offset < ZONA_TETRIS)
        return JUEGO_DINO;
    if (offset < ZONA_VIVORA)
        return JUEGO_TETRIS;
    if (offset < ZONA_ARKANOID)
        return JUEGO_VIVORA;
    return JUEGO_ARKANOID;
}

static const char *nombreJuego(int juego)
{
    switch (juego)
    {
    case JUEGO_TETRIS:
        return "TETRIS";
    case JUEGO_VIVORA:
        return "VIVORA";
    case JUEGO_ARKANOID:
        return "ARKANOID";
    default:
        return "DINO";
    }
}

// ========================================
// Menu: scroll infinito hasta que se presiona algun boton.
// ========================================
int mostrarMenuJuegos()
{
    const int filaSuperior = 18; // misma altura que usa PresentacionIset57
    int offset = 0;

    Serial.println("Menu: elegi un juego (cualquier boton selecciona el titulo en pantalla)");

    matrix.clearBuffer();

    while (true)
    {
        // el menu no tiene una duracion fija (espera a que aprieten un
        // boton), asi que la ola usa la hora absoluta: al ser periodica
        // da el mismo efecto sin necesitar un instante de arranque.
        aplicarBrilloOla(millis());

        matrix.clearBuffer();
        for (int x = 0; x < MATRIZ_ANCHO; x++)
        {
            uint8_t col = menuCols[(offset + x) % MENU_LEN];
            for (int fila = 0; fila < 7; fila++)
                if (col & (1 << fila))
                    matrix.set(x, filaSuperior - fila);
        }
        matrix.render();

        if (btnPressed(BTN_IZQUIERDA) || btnPressed(BTN_DERECHA) ||
            btnPressed(BTN_BAJAR) || btnPressed(BTN_ARRIBA))
        {
            matrix.setBrightness(BRILLO_NORMAL);
            matrix.clearBuffer();
            int juego = juegoEnOffset(offset);
            Serial.print("Menu: juego seleccionado: ");
            Serial.println(nombreJuego(juego));
            return juego;
        }

        offset = (offset + 1) % MENU_LEN;
        delay(120);
    }
}
