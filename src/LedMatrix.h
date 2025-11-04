/*
  LedMatrix.h
  Librería para controlar matriz de LEDs con MAX7219

  Hardware específico:
  - 10 displays MAX7219 en cascada (PAROLA_HW)
  - Matriz total de 14x25 LEDs
  - Columnas conectadas: 1-7 (col 0 no conectada)
  - Filas conectadas: 0-4 (filas 5-7 no usadas)

   Cada display MAX7219:
  ┌─────────────────────┐
  │    7 6 5 4 3 2 1 0  │  ← Columnas 7 a 1 (0 no conectada!)
  │  0 ● ● ● ● ● ● ● x  │  ← Fila 0
  │  1 ● ● ● ● ● ● ● x  │  ← Fila 1
  │  2 ● ● ● ● ● ● ● x  │  ← Fila 2
  │  3 ● ● ● ● ● ● ● x  │  ← Fila 3
  │  4 ● ● ● ● ● ● ● x  │  ← Fila 4
  └─────────────────────┘

  Disposición física de cada display:
  y=24 ->┌─────────┬─────────┐
         │ DISP 4  │ DISP 9  │
         ├─────────┼─────────┤
         │ DISP 3  │ DISP 8  │
         ├─────────┼─────────┤
         │ DISP 2  │ DISP 7  │
         ├─────────┼─────────┤
         │ DISP 1  │ DISP 6  │
         ├─────────┼─────────┤
         │ DISP 0  │ DISP 5  │
  y=0 -->└─────────┴─────────┘
          x=0................x=13

  Javier Rambaldo
  Octubre 2025
*/

#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <Arduino.h>
#include "MD_MAX72xx/MD_MAX72xx.h"

// ========================================
// Configuración del Hardware
// ========================================
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 10
#define CLK_PIN 18
#define DATA_PIN 23
#define CS_PIN 5

// Dimensiones de la matriz lógica
#define MATRIZ_ANCHO 14 // 14
#define MATRIZ_ALTO 25  // 25

class LedMatrix
{
private:
  MD_MAX72XX mx;

public:
  uint8_t mapa[MATRIZ_ALTO][MATRIZ_ANCHO]; // (y,x) buffer de pantalla.

  // Constructor
  LedMatrix() : mx(HARDWARE_TYPE, CS_PIN, MAX_DEVICES) {}

  // Inicializar la matriz
  void begin()
  {
    mx.begin();
    mx.clear();
    mx.control(MD_MAX72XX::INTENSITY, 8); // Brillo medio (0-15)
  }

  // Limpiar toda la matriz
  void clear()
  {
    mx.clear();
  }

  // Limpiar el buffer lógico
  void clearBuffer()
  {
    for (int y = 0; y < MATRIZ_ALTO; y++)
    {
      for (int x = 0; x < MATRIZ_ANCHO; x++)
      {
        mapa[y][x] = 0;
      }
    }
  }

  // copia el contenido del buffer logico al display.
  void render(bool invertY = false)
  {
    mx.clear();
    for (int y = 0; y < MATRIZ_ALTO; y++)
    {
      for (int x = 0; x < MATRIZ_ANCHO; x++)
      {
        if (mapa[y][x]) // solo enciendo porque ya esta todo apagado.
          setLED(x, invertY ? MATRIZ_ALTO - y - 1 : y, true);
      }
    }
  }

  // verifica el buffer logico en las coordenadas (x,y)
  inline bool isSet(uint8_t x, uint8_t y)
  {
    return mapa[y][x];
  }

  // dibuja un punto (on o off) en el buffer logico en las coordenadas (x,y)
  void plot(uint8_t x, uint8_t y, bool value)
  {
    mapa[y][x] = value;
  }

  // pone a 1 el buffer logico en las coordenadas (x,y)
  void set(uint8_t x, uint8_t y)
  {
    mapa[y][x] = 1;
  }

  // pone a 0 el buffer logico en las coordenadas (x,y)
  void clear(uint8_t x, uint8_t y)
  {
    mapa[y][x] = 0;
  }

  // copia el valor del buffer logico de (x2,y2) a (x1,y1)
  void copy(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
  {
    mapa[y1][x1] = mapa[y2][x2];
  }

  // Establecer brillo global (0-15)
  void setBrightness(uint8_t brillo)
  {
    mx.control(MD_MAX72XX::INTENSITY, brillo & 0xF);
  }

  // Encender/apagar un LED en coordenadas lógicas (x, y)
  // x: 0-13 (ancho)
  // y: 0-24 (alto, donde 0,0 es bottom,left)
  void setLED(int x, int y, bool encendido)
  {
    if (x < 0 || x >= MATRIZ_ANCHO || y < 0 || y >= MATRIZ_ALTO)
      return;

    int grupoVertical = y / 5; // Grupo de displays (0-4)
    int filaEnDisplay = y % 5; // Fila dentro del display (0-4)

    int displayNum;
    int colEnDisplay;

    if (x < 7)
    {
      // Columna izquierda (DISP 0-4)
      displayNum = grupoVertical;
      // Invertir: x=0 → col 7, x=1 → col 6, ..., x=6 → col 1
      colEnDisplay = 7 - x;
    }
    else
    {
      // Columna derecha (DISP 5-9)
      displayNum = 5 + grupoVertical;
      // Invertir: x=7 → col 7, x=8 → col 6, ..., x=13 → col 1
      colEnDisplay = 14 - x;
    }

    if (displayNum >= MAX_DEVICES)
      return;

    // Cada display ocupa 8 columnas virtuales
    int colGlobal = displayNum * 8 + colEnDisplay;

    // mx.setPoint(fila 0-4, columna 1-7, estado)
    mx.setPoint(filaEnDisplay, colGlobal, encendido);
  }

  // Dibujar un rectángulo
  void drawRect(int x, int y, int ancho, int alto, bool encendido)
  {
    for (int i = x; i < x + ancho; i++)
    {
      for (int j = y; j < y + alto; j++)
      {
        setLED(i, j, encendido);
      }
    }
  }

  // Dibujar una línea horizontal
  void drawHLine(int x, int y, int longitud, bool encendido)
  {
    for (int i = 0; i < longitud; i++)
    {
      setLED(x + i, y, encendido);
    }
  }

  // Dibujar una línea vertical
  void drawVLine(int x, int y, int longitud, bool encendido)
  {
    for (int i = 0; i < longitud; i++)
    {
      setLED(x, y + i, encendido);
    }
  }

  // Obtener dimensiones
  int getAncho() { return MATRIZ_ANCHO; }
  int getAlto() { return MATRIZ_ALTO; }
};

extern LedMatrix matrix;

#endif // LED_MATRIX_H