#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"
#include "TetrisGame.h"
#include "PresentacionIset57.h"
#include "DinoGame.h"
#include "SerpienteGame.h"
#include "ArkanoidGame.h"
#include "MenuJuegos.h"
#include "GameOverClasico.h"

static int juegoActual = JUEGO_DINO;

// muestra el menu, se queda con lo elegido y arranca ese juego.
static void elegirYArrancarJuego()
{
    juegoActual = mostrarMenuJuegos(); // DINO / TETRIS / VIVORA / ARKANOID desplazandose; un boton elige.

    switch (juegoActual)
    {
    case JUEGO_TETRIS:
        setupTetris(true);
        break;
    case JUEGO_VIVORA:
        setupSerpiente(true);
        break;
    case JUEGO_ARKANOID:
        setupArkanoid(true);
        break;
    default:
        setupDino(true);
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("*** MATRIX GAMES ISET 57 ***");

    matrix.begin();   // Inicializar matriz de LEDs
    setupControles(); // setup de los botones.

    mostrarPresentacion(10000); // ---> muestra el ISET 57

    elegirYArrancarJuego();
}

void loop()
{
    // 3 derrotas seguidas (en cualquier juego) ya mostraron el cartel
    // clasico de GAME OVER; ahora toca volver al menu a elegir de nuevo.
    if (hayPedidoDeMenu())
    {
        Serial.println("Volviendo al menu...");
        elegirYArrancarJuego();
        return;
    }

    switch (juegoActual)
    {
    case JUEGO_TETRIS:
        // si pones true se aborta el juego. Y esta preparado para un boton de Abort!
        loopTetris(false); 
        break;
    case JUEGO_VIVORA:
        loopSerpiente(false);
        break;
    case JUEGO_ARKANOID:
        loopArkanoid(false);
        break;
    default:
        loopDino(false);
        break;
    }
}
