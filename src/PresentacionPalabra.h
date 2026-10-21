#pragma once
#include <Arduino.h>

// Hace subir una palabra letra por letra desde abajo de la pantalla,
// cada letra 2 filas detras de la anterior, hasta que toda la tira
// desaparece arriba. `letras` es un array de glifos de 5 columnas
// (ver Fuente5x7.h: LETRA_X), uno por letra de la palabra.
void mostrarPalabraSubiendo(const uint8_t letras[][5], int numLetras);
