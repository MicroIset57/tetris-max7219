#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"
#include "TetrisGame.h"

void setup2()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("*** MATRIX GAMES ISET 57 ***");

    // Inicializar matriz de LEDs
    matrix.begin();
    setupControles(); // setup de los botones.

    // setupTetris(); // setup del juego Tetris (display, etc).

    for (int y = 0; y < MATRIZ_ALTO; y++)
    {
        matrix.setLED(0, y, true);
        delay(15);
    }

    //   matrix.setLED(0, 0, true);
    // matrix.setLED(13, 0, true);
    //  matrix.setLED(13, 24, true);
}

void loop2()
{
    bool abort = false;
    // loopTetris(abort);
}
