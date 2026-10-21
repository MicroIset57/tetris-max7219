/*
  Presentacion generica: una palabra sube letra por letra desde abajo
  de la pantalla hasta desaparecer arriba, con un efecto de ola de
  brillo de fondo. La usan las presentaciones de Tetris, Dino,
  Serpiente y Arkanoid, cada una con su propia palabra.

  Javier Rambaldo
  Octubre 2026.
*/

#include "LedMatrix.h"
#include "EfectoOla.h"
#include "PresentacionPalabra.h"

// bits (uno por columna, 5 en total) de la fila `fila` de una letra
// almacenada por columnas.
static uint8_t filaDeLetra(const uint8_t letra[5], int fila)
{
    uint8_t bits = 0;
    for (int col = 0; col < 5; col++)
        if (letra[col] & (1 << fila))
            bits |= (1 << col);
    return bits;
}

void mostrarPalabraSubiendo(const uint8_t letras[][5], int numLetras)
{
    const int altoLetra = 7;
    const int espacio = 2;                   // filas de separacion entre letras
    const int periodo = altoLetra + espacio; // filas que ocupa cada letra + su espacio
    const int totalFilas = numLetras * periodo;

    const int startX = (MATRIZ_ANCHO - 5) / 2; // centrada horizontalmente

    unsigned long inicio = millis();

    // topY = en que fila (buffer) esta la fila 0 de la tira completa.
    // arranca bien abajo (invisible) y sube hasta que hasta la ultima
    // letra termina de desaparecer arriba.
    for (int topY = -1; topY <= MATRIZ_ALTO + totalFilas; topY++)
    {
        aplicarBrilloOla(millis() - inicio);

        matrix.clearBuffer();
        for (int g = 0; g < totalFilas; g++)
        {
            int y = topY - g;
            if (y < 0 || y >= MATRIZ_ALTO)
                continue;

            int dentroDelBloque = g % periodo;
            if (dentroDelBloque >= altoLetra)
                continue; // cae en el espacio entre letras

            int letraIdx = g / periodo;
            uint8_t bits = filaDeLetra(letras[letraIdx], dentroDelBloque);
            for (int col = 0; col < 5; col++)
                if (bits & (1 << col))
                    matrix.set(startX + col, y);
        }
        matrix.render();
        delay(30);
    }

    matrix.setBrightness(BRILLO_NORMAL);
    matrix.clearBuffer();
}
