#pragma once

// ========================================
// Configuración de Pulsadores
// ========================================
#define BTN_IZQUIERDA 32
#define BTN_DERECHA 33
#define BTN_BAJAR 25
#define BTN_ROTAR 26
#define BTN_SALTAR BTN_ROTAR // boton unico del juego del Dino
#define BTN_ARRIBA BTN_ROTAR  // cuarta direccion (arriba) del juego de la Serpiente

void setupControles();
bool btnPressed(int pin);
