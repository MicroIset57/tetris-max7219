#include <Arduino.h>
#include "LedMatrix.h"

// ========================================
// Fuente 5x7 para la presentacion "ISET 57".
// Cada byte es una columna del caracter (5 columnas por letra);
// bit0 = fila superior del caracter, bit6 = fila inferior.
// Entre caracteres va una columna en 0 para separarlos.
// ========================================
// clang-format off
static const uint8_t presentacion[] = {
    0x41,    0x41,    0x7F,    0x41,    0x41,    0x00, // I
    0x46,    0x49,    0x49,    0x49,    0x31,    0x00, // S
    0x7F,    0x49,    0x49,    0x49,    0x41,    0x00, // E
    0x01,    0x01,    0x7F,    0x01,    0x01,    0x00, // T
    0x00,    0x00,    0x00,    0x00,                   // espacio
    0x4F,    0x49,    0x49,    0x49,    0x31,    0x00, // 5
    0x01,    0x01,    0x79,    0x05,    0x03,    0x00, // 7
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
        delay(60);
    }

    matrix.clearBuffer();
}
