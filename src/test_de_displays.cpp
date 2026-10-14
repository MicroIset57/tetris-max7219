#include <Arduino.h>
#include "LedMatrix.h"

// ========================================
// TEST DE LEDS: recorre la matriz encendiendo un solo LED por vez.
// Orden: de izquierda a derecha (x) y de abajo hacia arriba (y).
// ========================================

#define DEMORA_LED_MS 100 // tiempo que queda encendido cada LED
#define BRILLO_TEST 2     // brillo bajo (0-15) para minimizar consumo

void setup()
{
    // Lo primero: inicializar y apagar la matriz, ANTES de cualquier delay.
    // Tras un reset en caliente los MAX7219 conservan lo que tenían encendido,
    // y ese consumo durante el arranque del ESP32 puede provocar brownout.
    // begin() desactiva el modo test, borra todo y recién después saca a los
    // displays del modo shutdown, así que nunca se ven LEDs encendidos.
    // Serial.begin() no demora; lo uso para saber hasta dónde llega el arranque.
    Serial.begin(115200);
    Serial.println("\n[1] setup() iniciado, inicializando matriz...");

    matrix.begin();
    matrix.setBrightness(BRILLO_TEST);

    Serial.println("[2] matriz inicializada (deberia estar apagada)");
    delay(200);

    Serial.println("\n=================================");
    Serial.println("TEST DE LEDS: uno por uno");
    Serial.println("=================================\n");
}

void loop()
{
    for (int y = 0; y < MATRIZ_ALTO; y++)
    {
        for (int x = 0; x < MATRIZ_ANCHO; x++)
        {
            matrix.setLED(x, y, true);

            Serial.print("LED X=");
            Serial.print(x);
            Serial.print(" Y=");
            Serial.println(y);

            delay(DEMORA_LED_MS);
            matrix.setLED(x, y, false); // apago antes de pasar al siguiente
        }
    }

    Serial.println("\nRecorrido completo, reiniciando...\n");
    delay(1000);
}
