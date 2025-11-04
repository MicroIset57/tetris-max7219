# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Tetris game implementation for ESP32 microcontroller using a custom MAX7219 LED matrix display. The project controls 10 cascaded MAX7219 display modules arranged in a 14x25 LED matrix.

Originally developed in P5.js (2023/2024) and converted to C++ for ESP32 in 2025.

## Build & Development Commands

### Build and Upload
```bash
pio run -e esp32-tetris              # Build the project
pio run -e esp32-tetris -t upload    # Upload to ESP32
pio run -e esp32-tetris -t monitor   # Open serial monitor (115200 baud)
```

### Combined Commands
```bash
pio run -e esp32-tetris -t upload -t monitor   # Upload and monitor
```

## Hardware Architecture

### LED Matrix Configuration

- **10 MAX7219 displays** in cascade (PAROLA_HW type)
- **Physical matrix**: 14 columns x 25 rows
- **SPI Pins**: CLK=18, DATA=23, CS=5
- **Coordinate system**: (0,0) is bottom-left

### Display Layout
```
y=24 -> ┌─────────┬─────────┐
        │ DISP 4  │ DISP 9  │
        ├─────────┼─────────┤
        │ DISP 3  │ DISP 8  │
        ├─────────┼─────────┤
        │ DISP 2  │ DISP 7  │
        ├─────────┼─────────┤
        │ DISP 1  │ DISP 6  │
        ├─────────┼─────────┤
        │ DISP 0  │ DISP 5  │
y=0  -> └─────────┴─────────┘
        x=0................x=13
```

### Per-Display Wiring
- Each display uses **columns 1-7** (column 0 not connected)
- Each display uses **rows 0-4** (rows 5-7 not used)
- Cascade: ESP32 → DISP0 → DISP1 → ... → DISP9
- CLK and CS are shared in parallel to all displays

### Button Pins
- Left: GPIO 32
- Right: GPIO 33
- Down: GPIO 25
- Rotate: GPIO 26

All buttons use INPUT_PULLUP (active LOW).

## Code Architecture

### Core Components

**LedMatrix** ([src/LedMatrix.h](src/LedMatrix.h), [src/LedMatrix.cpp](src/LedMatrix.cpp))
- Abstraction layer over MD_MAX72XX library
- Manages coordinate transformation from logical (x,y) to physical display/row/column
- Provides buffered rendering via `mapa[MATRIZ_ALTO][MATRIZ_ANCHO]`
- Key methods: `setLED()`, `plot()`, `set()`, `clear()`, `render()`

**TetrisGame** ([src/TetrisGame.h](src/TetrisGame.h), [src/TetrisGame.cpp](src/TetrisGame.cpp))
- Game logic: tetrimino movement, collision detection, line clearing
- 7 tetrimino types with 4 rotations each (bit-packed as uint16_t)
- Uses logical buffer that gets inverted on Y-axis when rendering
- Currently has game logic partially commented out in main loop

**Controles** ([src/Controles.h](src/Controles.h), [src/Controles.cpp](src/Controles.cpp))
- Button debouncing with 150ms delay
- `btnPressed()` function for polling button state

### Coordinate System Gotcha

The display hardware has (0,0) at bottom-left, but tetrimino logic uses top-left origin. The `LedMatrix::setLED()` method handles the complex mapping:

1. Divides y into vertical groups (0-4) to select display number
2. Maps x to left column (DISP 0-4) or right column (DISP 5-9)
3. Inverts column numbering (x=0 → col 7, x=6 → col 1)

When rendering tetriminos, `dibujar_tetrimino()` inverts Y with `MATRIZ_ALTO - Y + a` to flip coordinate system.

### Rendering Pipeline

1. Game state stored in `LedMatrix::mapa[y][x]` buffer
2. Tetrimino drawn to buffer via `plot()`
3. `render()` copies entire buffer to hardware
4. Hardware coordinate transformation happens in `setLED()`

## External Dependencies

- **MD_MAX72XX**: Vendored in [src/MD_MAX72xx/](src/MD_MAX72xx/) - handles low-level MAX7219 SPI communication
- **FastLED**: Vendored in [lib/FastLED/](lib/FastLED/) - not actively used but present in codebase

## Current State

The main loop ([src/main.cpp](src/main.cpp)) has Tetris game logic commented out. Only basic LED testing is active. Uncomment `setupTetris()` and `loopTetris(abort)` to enable the game.

