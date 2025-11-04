/*
  TETRIS para matriz de LEDs con MAX7219 y ESP32
  --------------------------------------------
  Inspirado en el Tetris original de Alexey Pajitnov (1984)

  Año 2023/2024:
  Originalmente hice este programa en P5.js (javascript / processing)
  simulando cada led y la matriz completa de los displays.

  Año 2025:
  Adapté el código para que funcione en un ESP32 con una matriz de LEDs
  controlada por MAX7219, esto se lo pedí a la IA Claude para convertir códigos.

  Javier Rambaldo
  Octubre 2025.
*/

#include <Arduino.h>
#include "LedMatrix.h"
#include "Controles.h"

// ========================================
// Variables del Juego
// ========================================
int X = 8, Y = -1;
int rotacion = 0;
unsigned long tiempo = 0;
int tetriminoIndex = 0;
bool acomodalo = false;
int puntaje = 0;

// Forward declarations
bool detectarColision(int nuevoX, int nuevoY, int nuevaRotacion);
void nuevoTetrimino();
void limpiarLineasCompletas();

// ========================================
// TETRIMINOS (4 rotaciones cada uno)
// ========================================
const uint16_t tetriminos[7][4] = {
    {0b0000011000110000, 0b0010011001000000, 0b0000110001100000, 0b0000001001100100}, // O
    {0b0000001101100000, 0b0100011000100000, 0b0000011011000000, 0b0000010001100010}, // S
    {0b0100010001000100, 0b0000000011110000, 0b0010001000100010, 0b0000111100000000}, // I
    {0b0000011100100000, 0b0100011001000000, 0b0000010011100000, 0b0000001001100010}, // T
    {0b0000011101000000, 0b0100010001100000, 0b0000001011100000, 0b0000011000100010}, // L
    {0b0000011100010000, 0b0110010001000000, 0b0000100011100000, 0b0000001000100110}, // J
    {0b0000011001100000, 0b0000011001100000, 0b0000011001100000, 0b0000011001100000}  // Z
};

// ========================================
// Setup
// ========================================
void setupTetris()
{
    // Inicializar juego
    matrix.clearBuffer();
    randomSeed(analogRead(0));
    nuevoTetrimino();
    Serial.println("Tetris iniciado!");
}

// ========================================
// Animación de Game Over
// ========================================
void animacionGameOver()
{
    int separacion = 6;
    int velocidad = 35;

    for (int paso = 0; paso < MATRIZ_ALTO + separacion * 2; paso++)
    {
        matrix.clear();

        int barra1Y = paso;
        int barra2Y = paso + separacion;
        int barra3Y = paso + separacion * 2;

        // Dibujar mapa sin las partes borradas
        for (int y = 0; y < MATRIZ_ALTO; y++)
        {
            for (int x = 0; x < MATRIZ_ANCHO; x++)
            {
                bool borrado = (y > barra1Y) || (y > barra2Y) || (y > barra3Y);
                if (!borrado && matrix.isSet(x, y))
                {
                    matrix.setLED(x, y, true);
                }
            }
        }

        // Dibujar las 3 barras
        if (barra1Y >= 0 && barra1Y < MATRIZ_ALTO)
        {
            matrix.drawHLine(0, barra1Y, MATRIZ_ANCHO, true);
        }
        if (barra2Y >= 0 && barra2Y < MATRIZ_ALTO)
        {
            matrix.drawHLine(0, barra2Y, MATRIZ_ANCHO, true);
        }
        if (barra3Y >= 0 && barra3Y < MATRIZ_ALTO)
        {
            matrix.drawHLine(0, barra3Y, MATRIZ_ANCHO, true);
        }

        delay(velocidad);
    }

    matrix.clear();
    delay(300);
}

// ========================================
// Dibujar el mapa
// ========================================
// void dibujar_mapa_de_tetriminos()
// {
//     // el display tiene el 0,0 en bottom,left.
//     // por eso doy vuelta el mapa enn el display.
//     for (int y = 0; y < MATRIZ_ALTO; y++)
//     {
//         for (int x = 0; x < MATRIZ_ANCHO; x++)
//         {
//             if (matrix.isSet(x, y))                      // esta seteado en el mapa?
//                 matrix.setLED(x, MATRIZ_ALTO - y, true); // lo copio al display pero al reves.
//         }
//     }
// }

// ========================================
// Dibujar tetrimino actual
// ========================================
void dibujar_tetrimino()
{
    uint16_t tet = tetriminos[tetriminoIndex][rotacion];
    for (int a = 0; a < 4; a++)
    {
        for (int b = 0; b < 4; b++)
        {
            matrix.plot(X + b, MATRIZ_ALTO - Y + a, (tet & 1));
            // if (tet & 1)
            // {
            // copio al display pero al reves en Y
            // matrix.setLED(X + b, MATRIZ_ALTO - Y + a, true);
            // }
            tet >>= 1;
        }
    }
}

// ========================================
// Detectar colisión
// ========================================
bool detectarColision(int nuevoX, int nuevoY, int nuevaRot)
{
    uint16_t tet = tetriminos[tetriminoIndex][nuevaRot];
    for (int a = 0; a < 4; a++)
    {
        for (int b = 0; b < 4; b++)
        {
            if (tet & 1)
            {
                int xx = nuevoX + b;
                int yy = nuevoY + a;
                if (xx < 0 || xx >= MATRIZ_ANCHO || yy >= MATRIZ_ALTO)
                    return true;
                // if (yy >= 0 && matrix.isSet(xx, yy))
                //     return true;
            }
            tet >>= 1;
        }
    }
    return false;
}

// ========================================
// Fijar tetrimino
// ========================================
void fijarTetrimino()
{
    uint16_t tet = tetriminos[tetriminoIndex][rotacion];
    for (int a = 0; a < 4; a++)
    {
        for (int b = 0; b < 4; b++)
        {
            if (tet & 1)
            {
                int xx = X + b;
                int yy = Y + a;
                if (yy >= 0 && yy < MATRIZ_ALTO && xx >= 0 && xx < MATRIZ_ANCHO)
                {
                    matrix.set(xx, yy);
                }
            }
            tet >>= 1;
        }
    }
    limpiarLineasCompletas();
}

// ========================================
// Limpiar líneas completas
// ========================================
void limpiarLineasCompletas()
{
    int lineasEliminadas = 0;

    for (int y = MATRIZ_ALTO - 1; y >= 0; y--)
    {
        bool completa = true;
        for (int x = 0; x < MATRIZ_ANCHO; x++)
        {
            if (!matrix.isSet(x, y))
            {
                completa = false;
                break;
            }
        }

        if (completa)
        {
            for (int yy = y; yy > 0; yy--)
            {
                for (int x = 0; x < MATRIZ_ANCHO; x++)
                {
                    matrix.copy(x, yy, x, yy - 1);
                }
            }
            for (int x = 0; x < MATRIZ_ANCHO; x++)
            {
                matrix.clear(x, 0);
            }
            y++;
            lineasEliminadas++;
        }
    }

    if (lineasEliminadas > 0)
    {
        puntaje += lineasEliminadas * 100;
        Serial.print("Líneas: ");
        Serial.print(lineasEliminadas);
        Serial.print(" | Puntaje: ");
        Serial.println(puntaje);
    }
}

// ========================================
// Nuevo tetrimino
// ========================================
void nuevoTetrimino()
{
    rotacion = random(0, 4);
    tetriminoIndex = random(0, 7);
    X = random(3, 11);
    Y = -1;

    if (detectarColision(X, Y, rotacion))
    {
        Serial.println("GAME OVER!");
        Serial.print("Puntaje final: ");
        Serial.println(puntaje);

        animacionGameOver();

        matrix.clearBuffer();
        puntaje = 0;
        delay(1000);
    }
}

// ========================================
// Velocidad según puntaje
// ========================================
int velocidadPorPuntaje(int p)
{
    if (p < 100)
        return 500;
    if (p < 1000)
        return 400;
    if (p < 5000)
        return 300;
    if (p < 10000)
        return 200;
    if (p < 15000)
        return 100;
    if (p < 20000)
        return 50;
    return 50;
}

// ========================================
// Loop Principal
// ========================================
void loopTetris(bool abort)
{
    if (abort)
        return;

    // limpio el buffer interno para dibujar el frame nuevo:
    // matrix.clear();
    matrix.clearBuffer();

    // dibujar_mapa_de_tetriminos();
    dibujar_tetrimino();

    int vel = acomodalo ? 30 : velocidadPorPuntaje(puntaje);

    /*if (millis() > tiempo + vel)
    {
        tiempo = millis();
        if (!detectarColision(X, Y + 1, rotacion))
        {
            Y++;
        }
        else
        {
            // fijarTetrimino();
            nuevoTetrimino();
            acomodalo = false;
        }
    }*/

    // Controles
    // if (btnPressed(BTN_IZQUIERDA) && !detectarColision(X - 1, Y, rotacion))
    // {
    //     X--;
    // }
    // if (btnPressed(BTN_DERECHA) && !detectarColision(X + 1, Y, rotacion))
    // {
    //     X++;
    // }
    // if (btnPressed(BTN_BAJAR))
    // {
    //     acomodalo = true;
    // }
    // if (btnPressed(BTN_ROTAR))
    // {
    //     int r = (rotacion + 1) % 4;
    //     if (!detectarColision(X, Y, r))
    //     {
    //         rotacion = r;
    //     }
    // }

    matrix.render();
    delay(10);
}
