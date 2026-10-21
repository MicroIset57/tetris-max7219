/*
  DINO Google
  --------------------------------------------
  El juego del dinosaurio que salta cactus, para la matriz de LEDs.

  Pantalla apaisada: se gira 90° el eje logico de LedMatrix.
  - el ancho real del panel (14 px, MATRIZ_ANCHO) es la altura del juego (el salto).
  - el alto real del panel (25 px, MATRIZ_ALTO) es el largo del camino.
  El dino queda fijo en X y el cactus se acerca desde el otro extremo
  (como el original: dino a la izquierda, cactus entra por la derecha).

  Un solo boton: BTN_SALTAR.

  Javier Rambaldo
  Octubre 2026.
*/

#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"
#include "GameOverClasico.h"
#include "Fuente5x7.h"
#include "PresentacionPalabra.h"

// ========================================
// Variables del juego
// ========================================
const int LARGO = MATRIZ_ALTO; // 25, eje largo del camino

const int FILA_PISO = 0; // el piso vive en esta fila fija
const int FILA_BASE = 1; // el dino y el cactus arrancan una fila arriba del piso

const int DINO_X = 2;     // posicion fija del dino en el camino
const int ANCHO_DINO = 4; // ancho del sprite del dino (en el eje del camino)
const int ALTO_DINO = 5;  // alto del sprite del dino (en el eje de salto), patas incluidas

// tope de salto: que la cabeza del dino (fila mas alta) no se pase del panel.
const int ALTURA_MAX = (MATRIZ_ANCHO - 1) - FILA_BASE - (ALTO_DINO - 1);

const int LARGO_CACTUS = 2; // ancho maximo del cactus (tallo + brazo)

const int GRAVEDAD = 1;
const int IMPULSO_SALTO = 10;       // bien alto, para tener margen de sobra sobre el cactus mas alto
const int VELOCIDAD_CAIDA_MAX = -1; // la caida no acelera: siempre baja a este ritmo

// patron del piso: tramos de 8, 9 o 10 LEDs prendidos separados por 1 apagado.
const int PATRON_PISO_LEN = 60;

// fuente del dino: 4 columnas (ancho) x 5 filas (alto); bit0 = fila
// superior (cabeza), bit4 = fila inferior (pata). El cuerpo (filas 0-2)
// es igual en los dos cuadros; la pata es un solo LED en la ultima
// fila (con un hueco en la fila 3) que alterna de columna para animar
// la corrida.
static const uint8_t DINO_FRAME_A[ANCHO_DINO] =
    {0b00100,
     0b11100,
     0b01111,
     0b00011}; // pata abajo a la izquierda
static const uint8_t DINO_FRAME_B[ANCHO_DINO] =
    {0b00100,
     0b01100,
     0b11111,
     0b00011}; // pata abajo a la derecha

// static: son internas de este archivo, para no chocar con nombres
// parecidos (puntaje, etc.) de los otros juegos al linkear todo junto.
static int dinoY = 0;
static int dinoVelY = 0;
static bool piernaIzquierda = true;
static unsigned long tiempoAnimacion = 0;

static int cactusX = 0;
static int alturaCactus = 0;

static unsigned long puntaje = 0;
static bool gameOver = false;
static int scrollTick = 0;

static bool patronPiso[PATRON_PISO_LEN];

static unsigned long tiempoFisica = 0;
static unsigned long tiempoMovimiento = 0;

// Forward declarations
static void nuevoCactus();
static void animacionGameOverDino();

// ========================================
// Rotacion 90°: (x = a lo largo del camino, y = altura) -> buffer real.
// Se invierte el eje del camino para que el dino quede de un lado fijo
// y el cactus entre por el otro extremo, como el juego original.
// ========================================
static void plotDino(int x, int y, bool valor)
{
    matrix.plot(y, MATRIZ_ALTO - 1 - x, valor);
}

// ========================================
// Velocidad de scroll segun el puntaje (mas rapido = mas dificil)
// ========================================
static int velocidadPorPuntajeDino(unsigned long p)
{
    if (p < 50)
        return 150;
    if (p < 150)
        return 120;
    if (p < 300)
        return 100;
    if (p < 500)
        return 80;
    return 60;
}

// ========================================
// Piso: linea solida con huecos de 1 LED cada tanto, para que al
// correr se note el movimiento. Se arma una vez por partida con
// tramos de 8/9/10 LEDs prendidos y 1 apagado, y se hace scroll
// corriendo el indice con el que se lee.
// ========================================
static void generarPatronPiso()
{
    int i = 0;
    while (i < PATRON_PISO_LEN)
    {
        int tramo = random(8, 11); // 8, 9 o 10 prendidos
        for (int k = 0; k < tramo && i < PATRON_PISO_LEN; k++, i++)
            patronPiso[i] = true;
        if (i < PATRON_PISO_LEN)
            patronPiso[i++] = false; // 1 apagado
    }
}

static void dibujarPiso()
{
    for (int x = 0; x < LARGO; x++)
    {
        int idx = (x + scrollTick) % PATRON_PISO_LEN;
        if (patronPiso[idx])
            plotDino(x, FILA_PISO, true);
    }
}

// ========================================
// Dino: sprite de 4x5 (ver DINO_FRAME_A/B), parado una fila arriba
// del piso. La fila 0 de la fuente es la cabeza (arriba del todo) y
// la fila 4 es la pata (toca el piso).
// ========================================
static void dibujarDino()
{
    const uint8_t *cuadro = piernaIzquierda ? DINO_FRAME_A : DINO_FRAME_B;
    for (int col = 0; col < ANCHO_DINO; col++)
        for (int fila = 0; fila < ALTO_DINO; fila++)
            if (cuadro[col] & (1 << fila))
                plotDino(DINO_X + col, FILA_BASE + dinoY + (ALTO_DINO - 1 - fila), true);
}

// cactus: tallo de 1 de ancho en toda su altura, con un brazo a mitad
// de altura (si es suficientemente alto) para que se note la forma.
static void dibujarCactus()
{
    for (int h = 0; h < alturaCactus; h++)
        plotDino(cactusX, FILA_BASE + h, true);

    if (alturaCactus >= 3)
        plotDino(cactusX + 1, FILA_BASE + alturaCactus / 2, true);
}

// ========================================
// Colision: se superponen en el camino y el dino no llego lo
// suficientemente alto para pasar por arriba del cactus.
// ========================================
static bool detectarColisionDino()
{
    bool seSuperponen = cactusX < DINO_X + ANCHO_DINO && cactusX + LARGO_CACTUS > DINO_X;
    return seSuperponen && dinoY < alturaCactus;
}

// ========================================
// Nuevo cactus al final del camino, con altura al azar.
// ========================================
static void nuevoCactus()
{
    // nace justo dentro del borde para que entre entero (no se recorte
    // la segunda columna, que quedaria en LARGO, fuera de rango).
    cactusX = LARGO - LARGO_CACTUS;
    alturaCactus = random(2, 5); // 2, 3 o 4 pixeles de alto
}

// ========================================
// Presentacion: "DINO" sube desde abajo letra por letra (ver
// PresentacionPalabra.h).
// ========================================
static void mostrarPresentacionDino()
{
    static const uint8_t letras[][5] = {
        {LETRA_D},
        {LETRA_I},
        {LETRA_N},
        {LETRA_O},
    };
    mostrarPalabraSubiendo(letras, 4);
}

// ========================================
// Setup
// ========================================
void setupDino(bool mostrarPresentacion)
{
    if (mostrarPresentacion)
        mostrarPresentacionDino();

    matrix.clearBuffer();
    randomSeed(esp_random());

    dinoY = 0;
    dinoVelY = 0;
    piernaIzquierda = true;
    puntaje = 0;
    gameOver = false;
    scrollTick = 0;
    tiempoFisica = millis();
    tiempoMovimiento = millis();
    tiempoAnimacion = millis();

    generarPatronPiso();
    nuevoCactus();

    Serial.println("Dino iniciado!");
}

// ========================================
// Animacion de Game Over: parpadea la ultima escena.
// ========================================
static void animacionGameOverDino()
{
    Serial.print("GAME OVER! Puntaje: ");
    Serial.println(puntaje);

    for (int i = 0; i < 6; i++)
    {
        matrix.clear();
        delay(150);

        matrix.clearBuffer();
        dibujarPiso();
        dibujarDino();
        dibujarCactus();
        matrix.render();
        delay(150);
    }
}

// ========================================
// Loop Principal
// ========================================
void loopDino(bool abort)
{
    if (abort)
        return;

    if (gameOver)
    {
        // esperamos el boton para reiniciar
        if (btnPressed(BTN_SALTAR))
            setupDino(false);
        delay(10);
        return;
    }

    // salto: solo se puede saltar parado en el piso
    if (btnPressed(BTN_SALTAR) && dinoY == 0 && dinoVelY == 0)
        dinoVelY = IMPULSO_SALTO;

    // fisica del salto, en su propio tick (independiente del scroll)
    if (millis() - tiempoFisica > 80)
    {
        tiempoFisica = millis();
        if (dinoY > 0 || dinoVelY != 0)
        {
            dinoVelY -= GRAVEDAD;
            if (dinoVelY < VELOCIDAD_CAIDA_MAX)
                dinoVelY = VELOCIDAD_CAIDA_MAX; // no dejar que la caida acelere de mas
            dinoY += dinoVelY;

            if (dinoY <= 0)
            {
                dinoY = 0;
                dinoVelY = 0;
            }
            if (dinoY > ALTURA_MAX)
                dinoY = ALTURA_MAX;
        }
    }

    // animacion de correr: alterna la pata cada tanto
    if (millis() - tiempoAnimacion > 150)
    {
        tiempoAnimacion = millis();
        piernaIzquierda = !piernaIzquierda;
    }

    // scroll del camino: el cactus se acerca
    int velocidad = velocidadPorPuntajeDino(puntaje);
    if (millis() - tiempoMovimiento > (unsigned long)velocidad)
    {
        tiempoMovimiento = millis();
        cactusX--;
        scrollTick++;
        if (cactusX < 0)
        {
            puntaje += 10;
            nuevoCactus();
        }
    }

    if (detectarColisionDino())
    {
        gameOver = true;
        animacionGameOverDino();
        registrarDerrota();
        return;
    }

    matrix.clearBuffer();
    dibujarPiso();
    dibujarDino();
    dibujarCactus();
    matrix.render();

    delay(10);
}
