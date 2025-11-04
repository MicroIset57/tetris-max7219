#pragma once

// ========================================
// Configuración de Pulsadores
// ========================================
#define BTN_IZQUIERDA 32
#define BTN_DERECHA 33
#define BTN_BAJAR 25
#define BTN_ROTAR 26

void setupControles();
bool btnPressed(int pin);
