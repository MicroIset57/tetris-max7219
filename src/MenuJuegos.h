#pragma once

// Juegos seleccionables desde el menu, en el mismo orden en que se
// muestran sus titulos.
enum JuegoSeleccionado
{
    JUEGO_DINO = 0,
    JUEGO_TETRIS = 1,
    JUEGO_VIVORA = 2,
    JUEGO_ARKANOID = 3
};

// Muestra "DINO", "TETRIS", "VIVORA" y "ARKANOID" desplazandose de
// derecha a izquierda sin parar. Al presionar cualquier boton, se
// queda con el titulo que se esta mostrando en ese instante y lo
// devuelve.
int mostrarMenuJuegos();
