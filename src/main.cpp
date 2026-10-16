#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"
#include "TetrisGame.h"
#include "PresentacionIset57.h"
#include "DinoGame.h"

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("*** MATRIX GAMES ISET 57 ***");

    matrix.begin();             // Inicializar matriz de LEDs
    setupControles();           // setup de los botones.
    mostrarPresentacion(10000); // ---> muestra el ISET 57

    // setupTetris();              // setup del juego Tetris (display, etc).
    setupDino();
}

void loop()
{
    // loopTetris(false); // si pones true se aborta el juego.
    loopDino(false);
}
