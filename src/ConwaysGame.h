#pragma once

// Juego de la Vida de Conway: siembra una semilla al azar en toda la
// pantalla y la deja evolucionar sola (sin intervencion del jugador)
// hasta que se presiona cualquier boton. Si la generacion muere o
// llega a un estado estable, se vuelve a sembrar al azar.
//
// Se llama una unica vez al arrancar, antes de la presentacion ISET 57
// y del menu de juegos, asi que solo se ve justo despues de un reset.
void jugarConway();
