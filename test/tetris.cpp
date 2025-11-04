/*
  TETRIS para matriz de LEDs con MAX7219 y ESP32
  --------------------------------------------
  Inspirado en el Tetris original de Alexey Pajitnov (1984)

  Año 2023/2024:
  Originalmente hice este programa en P5.js (javascript / processing) simulando cada led y
  la matriz completa de los displays.

  Año 2025:
  Adapté el código para que funcione en un ESP32 con una matriz de LEDs controlada por MAX7219,
  esto se lo pedí a la IA Claude para convertir codigos.

  Javier Rambaldo

  Octubre 2025.
*/

#include <Arduino.h>
#include "MD_MAX72xx/MD_MAX72xx.h"

// ========================================
// Configuración del Hardware MAX7219
// ========================================
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES 10
#define CLK_PIN 18
#define DATA_PIN 23
#define CS_PIN 5

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// ========================================
// Configuración de Pulsadores
// ========================================
#define BTN_IZQUIERDA 32
#define BTN_DERECHA 33
#define BTN_BAJAR 25
#define BTN_ROTAR 26

// ========================================
// Configuración del Juego
// ========================================
const int pixels_ancho = 14;
const int pixels_alto = 25;

int X = 8, Y = -1;
int rotacion = 0;
unsigned long tiempo = 0;
int tetriminoIndex = 0;
bool acomodalo = false;
int puntaje = 0;

uint8_t mapa[25][14]; // [alto][ancho]

// forward declaration
bool detectarColision(int nuevoX, int nuevoY, int nuevaRotacion);
void nuevoTetrimino();

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
// Funciones de Control de Pulsadores
// ========================================
bool btnPressed(int pin)
{
  static unsigned long lastPress[4] = {0};
  static int pins[4] = {BTN_IZQUIERDA, BTN_DERECHA, BTN_BAJAR, BTN_ROTAR};

  int idx = 0;
  for (int i = 0; i < 4; i++)
  {
    if (pins[i] == pin)
    {
      idx = i;
      break;
    }
  }

  if (digitalRead(pin) == LOW && (millis() - lastPress[idx] > 150))
  {
    lastPress[idx] = millis();
    return true;
  }
  return false;
}

// ========================================
// Inicialización
// ========================================
void iniciarMapa()
{
  for (int y = 0; y < pixels_alto; y++)
  {
    for (int x = 0; x < pixels_ancho; x++)
    {
      mapa[y][x] = 0;
    }
  }
}

void setup()
{
  Serial.begin(115200);

  // Inicializar MAX7219
  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2);

  // Configurar pulsadores con pull-up interno
  pinMode(BTN_IZQUIERDA, INPUT_PULLUP);
  pinMode(BTN_DERECHA, INPUT_PULLUP);
  pinMode(BTN_BAJAR, INPUT_PULLUP);
  pinMode(BTN_ROTAR, INPUT_PULLUP);

  // Inicializar juego
  iniciarMapa();
  randomSeed(analogRead(0));
  nuevoTetrimino();

  Serial.println("Tetris iniciado!");
}

// ========================================
// Función para encender/apagar LED
// HARDWARE ESPECÍFICO:
// - Columnas conectadas: 1-7 (col 0 no conectada)
// - Filas conectadas: 0-4 (filas 5-7 no usadas)
// ========================================
void setLED(int x, int y, bool encendido)
{
  if (x < 0 || x >= pixels_ancho || y < 0 || y >= pixels_alto)
    return;

  // Invertir Y: y=0 debe aparecer abajo (fila física 24)
  int yInvertido = (pixels_alto - 1) - y;

  int grupoVertical = yInvertido / 5; // Grupo de displays (0-4)
  int filaEnDisplay = yInvertido % 5; // Fila dentro del display (0-4)

  int displayNum;
  int colEnDisplay;

  if (x < 7)
  {
    // Columna izquierda: displays 0-4
    displayNum = grupoVertical;
    // Invertir: x=0 → col 7, x=1 → col 6, ..., x=6 → col 1
    colEnDisplay = 7 - x;
  }
  else
  {
    // Columna derecha: displays 5-9
    displayNum = 5 + grupoVertical;
    // Invertir: x=7 → col 7, x=8 → col 6, ..., x=13 → col 1
    colEnDisplay = 7 - (x - 7);
  }

  if (displayNum >= MAX_DEVICES)
    return;

  // Cada display ocupa 8 columnas virtuales
  int colGlobal = displayNum * 8 + colEnDisplay;

  // mx.setPoint(fila 0-4, columna 1-7, estado)
  mx.setPoint(filaEnDisplay, colGlobal, encendido);
}

// ========================================
// Animación de Game Over
// 3 barras horizontales bajando y borrando
// NOTA: Las coordenadas Y están invertidas (y=0 abajo, y=24 arriba)
// ========================================
void animacionGameOver()
{
  int separacion = 6; // Separación entre barras
  int velocidad = 35; // Velocidad de bajada (ms)

  // Las barras bajan desde arriba (y=24) hasta abajo (y=0)
  for (int paso = 0; paso < pixels_alto + separacion * 2; paso++)
  {
    mx.clear();

    // Calcular posición de las 3 barras (empiezan desde arriba y=24)
    int barra1Y = 24 - paso;
    int barra2Y = 24 - paso + separacion;
    int barra3Y = 24 - paso + separacion * 2;

    // Dibujar el mapa actual (lo que va quedando sin borrar)
    for (int y = 0; y < pixels_alto; y++)
    {
      for (int x = 0; x < pixels_ancho; x++)
      {
        // Solo mostrar si no ha sido "borrado" por las barras
        // Las barras borran todo lo que está arriba de ellas (y > barraY)
        bool borrado = (y < barra1Y) || (y < barra2Y) || (y < barra3Y);

        if (!borrado && mapa[y][x])
        {
          setLED(x, y, true);
        }
      }
    }

    // Dibujar las 3 barras brillantes
    if (barra1Y >= 0 && barra1Y < pixels_alto)
    {
      for (int x = 0; x < pixels_ancho; x++)
      {
        setLED(x, barra1Y, true);
      }
    }

    if (barra2Y >= 0 && barra2Y < pixels_alto)
    {
      for (int x = 0; x < pixels_ancho; x++)
      {
        setLED(x, barra2Y, true);
      }
    }

    if (barra3Y >= 0 && barra3Y < pixels_alto)
    {
      for (int x = 0; x < pixels_ancho; x++)
      {
        setLED(x, barra3Y, true);
      }
    }

    delay(velocidad);
  }

  // Pantalla completamente borrada
  mx.clear();
  delay(300);
}

void animacionGameOver1()
{
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY);

  // Barrida de arriba hacia abajo
  for (int y = 0; y < pixels_alto; y++)
  {
    mx.clear();

    // Dibujar todas las filas hasta la actual
    for (int yy = 0; yy <= y; yy++)
    {
      for (int x = 0; x < pixels_ancho; x++)
      {
        setLED(x, yy, true);
      }
    }

    delay(40); // Velocidad de la barrida (ajustable)
  }

  // Mostrar brillo que se desvanece:
  for (int b = MAX_INTENSITY; b >= 0; b--)
  {
    mx.control(MD_MAX72XX::INTENSITY, b);
    delay(50);
  }

  // Limpiar todo
  mx.clear();
  delay(200);
  mx.control(MD_MAX72XX::INTENSITY, MAX_INTENSITY / 2); // Volver al brillo medio
}

// ========================================
// Dibujar el mapa de tetriminos fijados
// ========================================
void dibujar_mapa_de_tetriminos()
{
  for (int y = 0; y < pixels_alto; y++)
  {
    for (int x = 0; x < pixels_ancho; x++)
    {
      if (mapa[y][x] == 1)
      {
        setLED(x, y, true);
      }
    }
  }
}

// ========================================
// Dibujar el tetrimino actual
// ========================================
void dibujar_tetrimino()
{
  uint16_t tet = tetriminos[tetriminoIndex][rotacion];
  for (int a = 0; a < 4; a++)
  {
    for (int b = 0; b < 4; b++)
    {
      if (tet & 1)
      {
        setLED(X + b, Y + a, true);
      }
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
        if (xx < 0 || xx >= pixels_ancho || yy >= pixels_alto)
          return true;
        if (yy >= 0 && mapa[yy][xx])
          return true;
      }
      tet >>= 1;
    }
  }
  return false;
}

// ========================================
// Limpiar líneas completas
// ========================================
void limpiarLineasCompletas()
{
  int lineasEliminadas = 0;

  for (int y = pixels_alto - 1; y >= 0; y--)
  {
    bool completa = true;
    for (int x = 0; x < pixels_ancho; x++)
    {
      if (mapa[y][x] == 0)
      {
        completa = false;
        break;
      }
    }

    if (completa)
    {
      // Mover todas las filas hacia abajo
      for (int yy = y; yy > 0; yy--)
      {
        for (int x = 0; x < pixels_ancho; x++)
        {
          mapa[yy][x] = mapa[yy - 1][x];
        }
      }
      // Limpiar la fila superior
      for (int x = 0; x < pixels_ancho; x++)
      {
        mapa[0][x] = 0;
      }
      y++; // Revisar la misma fila nuevamente
      lineasEliminadas++;
    }
  }

  if (lineasEliminadas > 0)
  {
    puntaje += lineasEliminadas * 100;
    Serial.print("Líneas eliminadas: ");
    Serial.print(lineasEliminadas);
    Serial.print(" | Puntaje: ");
    Serial.println(puntaje);
  }
}

// ========================================
// Fijar tetrimino al mapa
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
        if (yy >= 0 && yy < pixels_alto && xx >= 0 && xx < pixels_ancho)
        {
          mapa[yy][xx] = 1;
        }
      }
      tet >>= 1;
    }
  }
  limpiarLineasCompletas();
}

// ========================================
// Crear nuevo tetrimino
// ========================================
void nuevoTetrimino()
{
  rotacion = random(0, 4);       // comienza al azar el giro
  tetriminoIndex = random(0, 7); // cualquier tetrimino al azar
  X = random(7, 9);              // al azar mas o menos al centro del tablero
  Y = -1;

  if (detectarColision(X, Y, rotacion))
  {
    Serial.println("GAME OVER!");
    Serial.print("Puntaje final: ");
    Serial.println(puntaje);

    // Ejecutar animación de Game Over
    animacionGameOver();

    // Reiniciar juego
    iniciarMapa();
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
void loop()
{
  // Limpiar pantalla
  mx.clear();

  // Dibujar mapa y tetrimino actual
  dibujar_mapa_de_tetriminos();
  dibujar_tetrimino();

  // Velocidad de caída
  int vel = acomodalo ? 30 : velocidadPorPuntaje(puntaje);

  if (millis() > tiempo + vel)
  {
    tiempo = millis();
    if (!detectarColision(X, Y + 1, rotacion))
    {
      Y++;
    }
    else
    {
      fijarTetrimino();
      nuevoTetrimino();
      acomodalo = false;
    }
  }

  // Controles con pulsadores
  if (btnPressed(BTN_IZQUIERDA) && !detectarColision(X - 1, Y, rotacion))
  {
    X--;
  }
  if (btnPressed(BTN_DERECHA) && !detectarColision(X + 1, Y, rotacion))
  {
    X++;
  }
  if (btnPressed(BTN_BAJAR))
  {
    acomodalo = true;
  }
  if (btnPressed(BTN_ROTAR))
  {
    int r = (rotacion + 1) % 4;
    if (!detectarColision(X, Y, r))
    {
      rotacion = r;
    }
  }

  delay(10); // Pequeño delay para estabilidad
}
