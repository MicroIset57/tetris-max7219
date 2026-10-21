/*
  ARKANOID (rompeparedes) para matriz de LEDs con MAX7219 y ESP32
  ----------------------------------------------------------------
  La pantalla ya es vertical (14 ancho x 25 alto), asi que no hace
  falta rotar nada: la paleta vive abajo y se mueve con
  izquierda/derecha, los ladrillos arrancan arriba y la pelota
  rebota entre medio. Al romper todos los ladrillos arranca un nivel
  nuevo (mas rapido); si la pelota pasa la paleta, se pierde.

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
const int FILA_PALETA = 1;   // la paleta vive en esta fila fija
const int PALETA_ANCHO = 3;
const int PALETA_PASO = 2;   // columnas que se mueve por cada apretada

const int FILAS_LADRILLOS = 6;
const int LADRILLOS_FILA_MAX = MATRIZ_ALTO - 2;                        // fila 23: deja la 24 libre como "techo"
const int LADRILLOS_FILA_MIN = LADRILLOS_FILA_MAX - FILAS_LADRILLOS + 1; // fila 18

// static: son internas de este archivo, para no chocar con nombres
// parecidos (puntaje, gameOver, etc.) de los otros juegos al linkear todo junto.
static bool ladrillos[FILAS_LADRILLOS][MATRIZ_ANCHO];
static int ladrillosRestantes = 0;

static int paletaX = 0;

static int ballX = 0, ballY = 0;
static int8_t ballDx = 1, ballDy = 1;

static int nivel = 0;
static unsigned long puntaje = 0;
static bool gameOver = false;
static unsigned long tiempoMovimiento = 0;

// Forward declarations
static void dibujarArkanoid();
static void animacionGameOverArkanoid();
static void nuevoNivel();

// ========================================
// Velocidad segun el nivel (cada nivel completado, mas rapido)
// ========================================
static int velocidadPorNivel(int nivel)
{
    int velocidad = 180 - nivel * 20;
    return velocidad < 60 ? 60 : velocidad;
}

// ========================================
// Pone la pelota arriba de la paleta, lista para el proximo nivel o
// para seguir jugando (no cuenta como derrota).
// ========================================
static void reposicionarPelota()
{
    ballX = paletaX + PALETA_ANCHO / 2;
    ballY = FILA_PALETA + 1;
    ballDx = random(0, 2) == 0 ? -1 : 1;
    ballDy = 1;
}

// ========================================
// Pared nueva de ladrillos, de punta a punta del panel.
// ========================================
static void nuevoNivel()
{
    ladrillosRestantes = FILAS_LADRILLOS * MATRIZ_ANCHO;
    for (int fila = 0; fila < FILAS_LADRILLOS; fila++)
        for (int col = 0; col < MATRIZ_ANCHO; col++)
            ladrillos[fila][col] = true;

    reposicionarPelota();
}

static void dibujarPaleta()
{
    for (int i = 0; i < PALETA_ANCHO; i++)
        matrix.set(paletaX + i, FILA_PALETA);
}

static void dibujarLadrillos()
{
    for (int fila = 0; fila < FILAS_LADRILLOS; fila++)
        for (int col = 0; col < MATRIZ_ANCHO; col++)
            if (ladrillos[fila][col])
                matrix.set(col, LADRILLOS_FILA_MIN + fila);
}

static void dibujarArkanoid()
{
    dibujarPaleta();
    dibujarLadrillos();
    matrix.set(ballX, ballY);
}

// ========================================
// Presentacion: "ARKANOID" sube desde abajo letra por letra (ver
// PresentacionPalabra.h).
// ========================================
static void mostrarPresentacionArkanoid()
{
    static const uint8_t letras[][5] = {
        {LETRA_A},
        {LETRA_R},
        {LETRA_K},
        {LETRA_A},
        {LETRA_N},
        {LETRA_O},
        {LETRA_I},
        {LETRA_D},
    };
    mostrarPalabraSubiendo(letras, 8);
}

// ========================================
// Setup
// ========================================
void setupArkanoid(bool mostrarPresentacion)
{
    if (mostrarPresentacion)
        mostrarPresentacionArkanoid();

    matrix.clearBuffer();
    randomSeed(esp_random());

    paletaX = (MATRIZ_ANCHO - PALETA_ANCHO) / 2;
    nivel = 0;
    puntaje = 0;
    gameOver = false;

    nuevoNivel();
    tiempoMovimiento = millis();

    Serial.println("Arkanoid iniciado!");
}

// ========================================
// Animacion de Game Over: parpadea la ultima escena.
// ========================================
static void animacionGameOverArkanoid()
{
    Serial.print("GAME OVER! Puntaje: ");
    Serial.println(puntaje);

    for (int i = 0; i < 6; i++)
    {
        matrix.clear();
        delay(150);

        matrix.clearBuffer();
        dibujarArkanoid();
        matrix.render();
        delay(150);
    }
}

// ========================================
// Animacion de nivel completado: un par de parpadeos y arranca el
// siguiente (no es una derrota, asi que no toca registrarDerrota()).
// ========================================
static void animacionNivelCompletado()
{
    Serial.print("Nivel completado! Puntaje: ");
    Serial.println(puntaje);

    for (int i = 0; i < 3; i++)
    {
        matrix.clear();
        delay(120);

        matrix.clearBuffer();
        dibujarPaleta();
        matrix.render();
        delay(120);
    }

    nivel++;
    nuevoNivel();
}

// ========================================
// Loop Principal
// ========================================
void loopArkanoid(bool abort)
{
    if (abort)
        return;

    if (gameOver)
    {
        // esperamos cualquiera de los dos botones para reiniciar
        if (btnPressed(BTN_IZQUIERDA) || btnPressed(BTN_DERECHA))
            setupArkanoid(false);
        delay(10);
        return;
    }

    if (btnPressed(BTN_IZQUIERDA))
        paletaX -= PALETA_PASO;
    if (btnPressed(BTN_DERECHA))
        paletaX += PALETA_PASO;

    if (paletaX < 0)
        paletaX = 0;
    if (paletaX > MATRIZ_ANCHO - PALETA_ANCHO)
        paletaX = MATRIZ_ANCHO - PALETA_ANCHO;

    int velocidad = velocidadPorNivel(nivel);
    if (millis() - tiempoMovimiento > (unsigned long)velocidad)
    {
        tiempoMovimiento = millis();

        int nextX = ballX + ballDx;
        int nextY = ballY + ballDy;

        // paredes laterales
        if (nextX < 0 || nextX >= MATRIZ_ANCHO)
        {
            ballDx = -ballDx;
            nextX = ballX + ballDx;
        }

        // techo
        if (nextY >= MATRIZ_ALTO)
        {
            ballDy = -ballDy;
            nextY = ballY + ballDy;
        }

        // ladrillos: si el proximo paso cae en uno, se rompe y la
        // pelota rebota en vez de entrar en esa celda.
        if (nextY >= LADRILLOS_FILA_MIN && nextY <= LADRILLOS_FILA_MAX &&
            ladrillos[nextY - LADRILLOS_FILA_MIN][nextX])
        {
            ladrillos[nextY - LADRILLOS_FILA_MIN][nextX] = false;
            ladrillosRestantes--;
            puntaje += 10;
            ballDy = -ballDy;
            nextY = ballY + ballDy;
        }

        // paleta: solo importa cuando la pelota esta bajando y llega
        // a su fila (o mas abajo, si se salto un paso).
        if (ballDy < 0 && nextY <= FILA_PALETA)
        {
            if (nextX >= paletaX && nextX < paletaX + PALETA_ANCHO)
            {
                ballDy = 1;
                nextY = FILA_PALETA + 1;

                // el lugar donde pega define el angulo de salida.
                int mitad = paletaX + PALETA_ANCHO / 2;
                if (nextX < mitad)
                    ballDx = -1;
                else if (nextX > mitad)
                    ballDx = 1;
            }
            else
            {
                gameOver = true;
            }
        }

        if (gameOver)
        {
            animacionGameOverArkanoid();
            registrarDerrota();
            return;
        }

        ballX = nextX;
        ballY = nextY;

        if (ladrillosRestantes <= 0)
        {
            matrix.clearBuffer();
            dibujarArkanoid();
            matrix.render();
            animacionNivelCompletado();
            return;
        }
    }

    matrix.clearBuffer();
    dibujarArkanoid();
    matrix.render();

    delay(10);
}
