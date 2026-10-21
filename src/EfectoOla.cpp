/*
  Efecto de "ola" de brillo (ver EfectoOla.h).

  Javier Rambaldo
  Octubre 2026.
*/

#include "LedMatrix.h"
#include "EfectoOla.h"

void aplicarBrilloOla(unsigned long transcurridoMs, unsigned long periodoMs,
                       uint8_t brilloMin, uint8_t brilloMax)
{
    float fase = (2.0f * PI * transcurridoMs) / periodoMs;
    float t = (sinf(fase) + 1.0f) / 2.0f; // 0..1

    uint8_t brillo = brilloMin + (uint8_t)roundf(t * (brilloMax - brilloMin));
    matrix.setBrightness(brillo);
}
