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

const int pixels_ancho = 14;
const int pixels_alto = 25;

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

    int grupoVertical = y / 5; // Grupo de displays (0-4)
    int filaEnDisplay = y % 5; // Fila dentro del display (0-4)

    // Determino si es la columna de displays derecha o izquierda.
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
        colEnDisplay = 14 - x;
    }

    if (displayNum >= MAX_DEVICES)
        return;

    // Cada display ocupa 8 columnas virtuales
    int colGlobal = displayNum * 8 + colEnDisplay;

    // mx.setPoint(fila 0-4, columna 1-7, estado)
    mx.setPoint(filaEnDisplay, colGlobal, encendido);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    mx.begin();
    mx.clear();
    mx.control(MD_MAX72XX::INTENSITY, 8);

    Serial.println("\n=================================");
    Serial.println("TEST DE LEDS MAX7219");
    Serial.println("=================================\n");
}

void loop()
{

    // ========================================
    // TEST 7: Patrón de ajedrez
    // ========================================
    for (size_t i = 0; i < 10; i++)
    {

        Serial.println("\nTEST 7: Patrón de ajedrez...");
        mx.clear();
        for (int y = 0; y < pixels_alto; y++)
        {
            for (int x = 0; x < pixels_ancho; x++)
            {
                if ((x + y) % 2 == (i%2==0))
                {
                    setLED(x, y, true);
                }
            }
        }
        delay(1000);
    }

    // ========================================
    // TEST 1: Encender todos los LEDs
    // ========================================
    // Serial.println("TEST 1: Encendiendo TODOS los LEDs...");
    // mx.clear();
    // for (int y = 0; y < pixels_alto; y++)
    // {
    //     for (int x = 0; x < pixels_ancho; x++)
    //     {
    //         setLED(x, y, true);
    //     }
    // }
    // delay(3000);

    // // ========================================
    // // TEST 2: Encender display por display
    // // ========================================
    // Serial.println("\nTEST 2: Encender cada display individualmente...");
    // for (int disp = 0; disp < MAX_DEVICES; disp++)
    // {
    //     mx.clear();
    //     Serial.print("Display ");
    //     Serial.println(disp);

    //     // Encender todo el display actual (filas 0-4, columnas 1-7)
    //     for (int row = 0; row < 5; row++)
    //     {
    //         for (int col = 1; col <= 7; col++)
    //         {
    //             mx.setPoint(row, disp * 8 + col, true);
    //         }
    //     }
    //     delay(1000);
    // }

    // // ========================================
    // // TEST 3: Barrido horizontal (columna por columna)
    // // ========================================
    // Serial.println("\nTEST 3: Barrido horizontal (columna por columna)...");
    // mx.clear();
    // for (int x = 0; x < pixels_ancho; x++)
    // {
    //     mx.clear();
    //     Serial.print("Columna juego: ");
    //     Serial.println(x);

    //     for (int y = 0; y < pixels_alto; y++)
    //     {
    //         setLED(x, y, true);
    //     }
    //     delay(500);
    // }

    // // ========================================
    // // TEST 4: Barrido vertical (fila por fila)
    // // ========================================
    // Serial.println("\nTEST 4: Barrido vertical (fila por fila)...");
    // mx.clear();
    // for (int y = 0; y < pixels_alto; y++)
    // {
    //     mx.clear();
    //     Serial.print("Fila juego: ");
    //     Serial.println(y);

    //     for (int x = 0; x < pixels_ancho; x++)
    //     {
    //         setLED(x, y, true);
    //     }
    //     delay(300);
    // }

    // ========================================
    // TEST 5: Encender LED individual recorriendo toda la matriz
    // ========================================
    Serial.println("\nTEST 5: LED individual (uno por uno)...");
    mx.clear();
    for (int y = 0; y < pixels_alto; y++)
    {
        for (int x = 0; x < pixels_ancho; x++)
        {
            mx.clear();
            setLED(x, y, true);
            Serial.print("LED en X=");
            Serial.print(x);
            Serial.print(" Y=");
            Serial.println(y);
            delay(100);
        }
    }

    // ========================================
    // TEST 6: Prueba de mapeo directo (sin función setLED)
    // ========================================
    Serial.println("\nTEST 6: Mapeo directo MAX7219...");
    Serial.println("HARDWARE: Columnas 1-7 (col 0 sin conectar), Filas 0-4");
    mx.clear();

    Serial.println("\nEncendiendo fila 0 de cada display (usando col 1-7)...");
    for (int disp = 0; disp < MAX_DEVICES; disp++)
    {
        for (int col = 1; col <= 7; col++)
        { // Solo columnas 1-7
            mx.setPoint(0, disp * 8 + col, true);
        }
    }
    delay(2000);

    Serial.println("Encendiendo columna 1 de todos los displays (usando filas 0-4)...");
    mx.clear();
    for (int disp = 0; disp < MAX_DEVICES; disp++)
    {
        for (int row = 0; row < 5; row++)
        { // Solo filas 0-4
            mx.setPoint(row, disp * 8 + 1, true);
        }
    }
    delay(2000);

    Serial.println("\nProbando esquinas de cada display...");
    mx.clear();
    for (int disp = 0; disp < MAX_DEVICES; disp++)
    {
        int colBase = disp * 8;
        // Esquina superior izquierda (fila 0, col 1)
        mx.setPoint(0, colBase + 1, true);
        // Esquina superior derecha (fila 0, col 7)
        mx.setPoint(0, colBase + 7, true);
        // Esquina inferior izquierda (fila 4, col 1)
        mx.setPoint(4, colBase + 1, true);
        // Esquina inferior derecha (fila 4, col 7)
        mx.setPoint(4, colBase + 7, true);
    }
    delay(3000);

    // ========================================
    // TEST 8: Marco perimetral
    // ========================================
    Serial.println("\nTEST 8: Marco perimetral...");
    mx.clear();
    // Líneas horizontales
    for (int x = 0; x < pixels_ancho; x++)
    {
        setLED(x, 0, true);               // Fila superior
        setLED(x, pixels_alto - 1, true); // Fila inferior
    }
    // Líneas verticales
    for (int y = 0; y < pixels_alto; y++)
    {
        setLED(0, y, true);                // Columna izquierda
        setLED(pixels_ancho - 1, y, true); // Columna derecha
    }
    delay(3000);

    // ========================================
    // TEST 9: Cruz central
    // ========================================
    Serial.println("\nTEST 9: Cruz central...");
    mx.clear();
    int centroX = pixels_ancho / 2;
    int centroY = pixels_alto / 2;

    // Línea vertical
    for (int y = 0; y < pixels_alto; y++)
    {
        setLED(centroX, y, true);
    }
    // Línea horizontal
    for (int x = 0; x < pixels_ancho; x++)
    {
        setLED(x, centroY, true);
    }
    delay(3000);

    mx.clear();
    Serial.println("\n=================================");
    Serial.println("Ciclo de tests completado");
    Serial.println("Reiniciando en 2 segundos...");
    Serial.println("=================================\n");
    delay(2000);
}