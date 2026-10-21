#include <Arduino.h>
#include "LedMatrix.h"
#include "EfectoOla.h"
#include "Fuente5x7.h"

// ========================================
// "ISET 57", con el diccionario de letras compartido (Fuente5x7.h).
// ========================================
// clang-format off
static const uint8_t presentacion[] = {
    LETRA_I, SEP, LETRA_S, SEP, LETRA_E, SEP, LETRA_T, SEP,
    SEP, SEP, SEP, SEP,                   // espacio
    LETRA_5, SEP, LETRA_7, SEP,
    SEP, SEP, SEP, SEP,                   // espacio
};
static const int PRESENTACION_LEN = sizeof(presentacion) / sizeof(presentacion[0]);
// clang-format on

// ========================================
// Presentacion inicial: "ISET 57" recorriendo la pantalla
// de derecha a izquierda durante `duracionMs`.
// ========================================
void mostrarPresentacion(unsigned long duracionMs)
{
    const int filaSuperior = 18; // fila superior de la fuente (0 abajo, 24 arriba)
    unsigned long inicio = millis();
    int offset = 0;

    while (millis() - inicio < duracionMs)
    {
        aplicarBrilloOla(millis() - inicio);

        matrix.clearBuffer();
        for (int x = 0; x < MATRIZ_ANCHO; x++)
        {
            uint8_t col = presentacion[(offset + x) % PRESENTACION_LEN];
            for (int fila = 0; fila < 7; fila++)
            {
                if (col & (1 << fila))
                    matrix.set(x, filaSuperior - fila);
            }
        }
        matrix.render();
        offset = (offset + 1) % PRESENTACION_LEN;
        delay(50);
    }

    matrix.setBrightness(BRILLO_NORMAL);
    matrix.clearBuffer();
}
