# Tetris con Max7219

## Circuito

![Circuito de cada display](doc/Schematic_Display-Max7219-PRODUCCCION_2025-09-24.png)

## Matriz de displays

    -------------------------------------------------------------------
    [Display 0][Display 1]  <- 14 cols x 5 filas (filas 0-4 del juego)
    [Display 2][Display 3]  <- 14 cols x 5 filas (filas 5-9 del juego)
    [Display 4][Display 5]  <- 14 cols x 5 filas (filas 10-14 del juego)
    [Display 6][Display 7]  <- 14 cols x 5 filas (filas 15-19 del juego)
    [Display 8][Display 9]  <- 14 cols x 5 filas (filas 20-24 del juego)
    -------------------------------------------------------------------
    
    ┌─────────┬─────────┐
    │ DISP 0  │ DISP 1  │ ← Filas 0-4 del juego
    │  (7x5)  │  (7x5)  │   Total: 14 cols x 5 filas
    ├─────────┼─────────┤
    │ DISP 2  │ DISP 3  │ ← Filas 5-9 del juego
    │  (7x5)  │  (7x5)  │
    ├─────────┼─────────┤
    │ DISP 4  │ DISP 5  │ ← Filas 10-14 del juego
    │  (7x5)  │  (7x5)  │
    ├─────────┼─────────┤
    │ DISP 6  │ DISP 7  │ ← Filas 15-19 del juego
    │  (7x5)  │  (7x5)  │
    ├─────────┼─────────┤
    │ DISP 8  │ DISP 9  │ ← Filas 20-24 del juego
    │  (7x5)  │  (7x5)  │
    └─────────┴─────────┘

## Conexión en cascada:
    ESP32 → DISP0 → DISP1 → DISP2 → DISP3 → DISP4 → DISP5 → DISP6 → DISP7 → DISP8 → DISP9

    DIN del ESP32 (GPIO23) → DIN del DISP0
    DOUT del DISP0 → DIN del DISP1
    DOUT del DISP1 → DIN del DISP2
    ... y así sucesivamente

    CLK y CS van a TODOS en paralelo

    