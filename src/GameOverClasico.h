#pragma once

// Cada juego llama a esta funcion cuando pierde. Cuenta las derrotas
// seguidas (en cualquier juego) y, a la tercera, muestra el cartel
// clasico de "GAME OVER" y deja pendiente el pedido de volver al
// menu principal.
void registrarDerrota();

// true (una sola vez) si hay que volver al menu porque se acaban de
// cumplir 3 derrotas seguidas. main.cpp la consulta en cada vuelta
// del loop(), antes de seguir con el juego actual.
bool hayPedidoDeMenu();
