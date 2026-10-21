/*
  Serpiente para matriz de LEDs con MAX7219 y ESP32
  -------------------------------------------------
  La serpiente arranca en una posicion y direccion al azar. El jugador
  la dirige con los 4 botones (izquierda/derecha/abajo/arriba) buscando
  la comida (un pixel al azar); cada vez que la come, se alarga y
  aparece una comida nueva. Los bordes no matan: si sale de la grilla
  reaparece del lado opuesto. Solo pierde si choca contra su propia cola.

  Javier Rambaldo
  Octubre 2025.
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
const int LONGITUD_INICIAL = 3;
const int MAX_SERPIENTE_LEN = MATRIZ_ANCHO * MATRIZ_ALTO; // tope teorico: toda la grilla

// static: son internas de este archivo, para no chocar con nombres
// parecidos (puntaje, gameOver, etc.) de los otros juegos al linkear todo junto.
static uint8_t cuerpoX[MAX_SERPIENTE_LEN];
static uint8_t cuerpoY[MAX_SERPIENTE_LEN];
static int longitud = LONGITUD_INICIAL;

// direccion actual de movimiento y la pedida por el jugador (se aplica
// en el siguiente tick, para no permitir mas de un giro por paso).
static int8_t dx = 1, dy = 0;
static int8_t sigDx = 1, sigDy = 0;

static int comidaX = 0, comidaY = 0;

static unsigned long puntaje = 0;
static bool gameOver = false;
static unsigned long tiempoMovimiento = 0;

// Forward declarations
static void dibujarSerpiente();
static void animacionGameOverSerpiente();

// ========================================
// true si (x,y) esta entre los primeros "limite" segmentos del cuerpo
// ========================================
static bool enCuerpo(int x, int y, int limite)
{
    for (int i = 0; i < limite; i++)
        if (cuerpoX[i] == x && cuerpoY[i] == y)
            return true;
    return false;
}

// ========================================
// Comida: un pixel al azar que no caiga sobre la serpiente
// ========================================
static void nuevaComida()
{
    int x, y;
    do
    {
        x = random(0, MATRIZ_ANCHO);
        y = random(0, MATRIZ_ALTO);
    } while (enCuerpo(x, y, longitud));

    comidaX = x;
    comidaY = y;
}

// ========================================
// Velocidad segun el largo (mas larga = mas rapida = mas dificil)
// ========================================
static int velocidadPorLongitud(int longitud)
{
    if (longitud < 8)
        return 180;
    if (longitud < 15)
        return 150;
    if (longitud < 25)
        return 120;
    if (longitud < 40)
        return 100;
    if (longitud < 60)
        return 85;
    return 70;
}

// ========================================
// Avanza la cabeza a (nuevaX, nuevaY). Si "crecio", la cola no se
// descarta (el cuerpo se alarga); si no, se corre todo un lugar.
// ========================================
static void moverSerpiente(int nuevaX, int nuevaY, bool crecio)
{
    if (crecio && longitud >= MAX_SERPIENTE_LEN)
        crecio = false; // ya no entra mas serpiente en la grilla

    int limite = crecio ? longitud : longitud - 1;
    for (int i = limite; i > 0; i--)
    {
        cuerpoX[i] = cuerpoX[i - 1];
        cuerpoY[i] = cuerpoY[i - 1];
    }

    cuerpoX[0] = nuevaX;
    cuerpoY[0] = nuevaY;

    if (crecio)
        longitud++;
}

static void dibujarSerpiente()
{
    for (int i = 0; i < longitud; i++)
        matrix.set(cuerpoX[i], cuerpoY[i]);

    matrix.set(comidaX, comidaY);
}

// ========================================
// Presentacion: "VIVORA" sube desde abajo letra por letra (ver
// PresentacionPalabra.h).
// ========================================
static void mostrarPresentacionSerpiente()
{
    static const uint8_t letras[][5] = {
        {LETRA_V},
        {LETRA_I},
        {LETRA_V},
        {LETRA_O},
        {LETRA_R},
        {LETRA_A},
    };
    mostrarPalabraSubiendo(letras, 6);
}

// ========================================
// Setup
// ========================================
void setupSerpiente(bool mostrarPresentacion)
{
    if (mostrarPresentacion)
        mostrarPresentacionSerpiente();

    matrix.clearBuffer();
    randomSeed(esp_random());

    gameOver = false;
    puntaje = 0;

    // direccion inicial al azar (una de las 4 cardinales)
    switch (random(0, 4))
    {
    case 0:
        dx = 1;
        dy = 0;
        break; // derecha
    case 1:
        dx = -1;
        dy = 0;
        break; // izquierda
    case 2:
        dx = 0;
        dy = 1;
        break; // arriba
    default:
        dx = 0;
        dy = -1;
        break; // abajo
    }
    sigDx = dx;
    sigDy = dy;

    // cabeza en una posicion al azar, con margen para no nacer pegada a un borde
    int cabezaX = random(3, MATRIZ_ANCHO - 3);
    int cabezaY = random(3, MATRIZ_ALTO - 3);

    longitud = LONGITUD_INICIAL;
    for (int i = 0; i < longitud; i++)
    {
        cuerpoX[i] = cabezaX - dx * i;
        cuerpoY[i] = cabezaY - dy * i;
    }

    nuevaComida();
    tiempoMovimiento = millis();

    Serial.println("Serpiente iniciada!");
}

// ========================================
// Animacion de Game Over: parpadea la ultima escena.
// ========================================
static void animacionGameOverSerpiente()
{
    Serial.print("GAME OVER! Puntaje: ");
    Serial.println(puntaje);

    for (int i = 0; i < 6; i++)
    {
        matrix.clear();
        delay(150);

        matrix.clearBuffer();
        dibujarSerpiente();
        matrix.render();
        delay(150);
    }
}

// ========================================
// Loop Principal
// ========================================
void loopSerpiente(bool abort)
{
    if (abort)
        return;

    if (gameOver)
    {
        // esperamos cualquier boton para reiniciar
        if (btnPressed(BTN_IZQUIERDA) || btnPressed(BTN_DERECHA) || btnPressed(BTN_BAJAR) || btnPressed(BTN_ARRIBA))
            setupSerpiente(false);
        delay(10);
        return;
    }

    // direccion pedida por el jugador: no se puede invertir sobre la
    // propia cola, asi que se ignora el giro que vuelve para atras.
    if (btnPressed(BTN_IZQUIERDA) && dx != 1)
    {
        sigDx = -1;
        sigDy = 0;
    }
    if (btnPressed(BTN_DERECHA) && dx != -1)
    {
        sigDx = 1;
        sigDy = 0;
    }
    if (btnPressed(BTN_BAJAR) && dy != 1)
    {
        sigDx = 0;
        sigDy = -1;
    }
    if (btnPressed(BTN_ARRIBA) && dy != -1)
    {
        sigDx = 0;
        sigDy = 1;
    }

    int velocidad = velocidadPorLongitud(longitud);
    if (millis() - tiempoMovimiento > (unsigned long)velocidad)
    {
        tiempoMovimiento = millis();

        dx = sigDx;
        dy = sigDy;

        // el borde no mata: si se sale, reaparece del lado opuesto
        int nuevaX = (cuerpoX[0] + dx + MATRIZ_ANCHO) % MATRIZ_ANCHO;
        int nuevaY = (cuerpoY[0] + dy + MATRIZ_ALTO) % MATRIZ_ALTO;

        bool comida = (nuevaX == comidaX && nuevaY == comidaY);
        int limiteColision = comida ? longitud : longitud - 1;

        if (enCuerpo(nuevaX, nuevaY, limiteColision))
        {
            gameOver = true;
        }
        else
        {
            moverSerpiente(nuevaX, nuevaY, comida);
            if (comida)
            {
                puntaje++;
                nuevaComida();
            }
        }

        if (gameOver)
        {
            animacionGameOverSerpiente();
            registrarDerrota();
            return;
        }
    }

    matrix.clearBuffer();
    dibujarSerpiente();
    matrix.render();

    delay(10);
}
