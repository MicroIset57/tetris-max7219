#pragma once
#include <Arduino.h>

// Brillo por defecto fuera de la ola (el mismo que pone LedMatrix::begin()).
const uint8_t BRILLO_NORMAL = 3;

// Efecto de "ola": hace subir y bajar el brillo global de toda la
// matriz en un vaiven continuo (como una respiracion), para darle
// vida al scroll de letras en las presentaciones y en el cartel de
// GAME OVER. Se basa en el tiempo transcurrido (no en los frames), asi
// da el mismo ritmo sin importar cada cuanto se llama.
void aplicarBrilloOla(unsigned long transcurridoMs, unsigned long periodoMs = 900,
                      uint8_t brilloMin = 1, uint8_t brilloMax = 12);
