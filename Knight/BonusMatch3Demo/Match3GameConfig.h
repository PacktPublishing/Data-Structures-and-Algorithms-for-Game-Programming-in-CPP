#pragma once

#include "Knight.h"

static const int COLS = 8;
static const int ROWS = 8;

// Size of the tile (cell) and board
static const int CELL = 128;
static const int PADDING = 24;
static const int BOARD_W = COLS * CELL;
static const int BOARD_H = ROWS * CELL;
static const int SCREEN_W = BOARD_W + PADDING * 2;
static const int SCREEN_H = BOARD_H + PADDING * 2;
static const int CHIP_TYPES = 5;

// Animation timing
static const float SWAP_TIME = 0.15f;
static const float FALL_SPEED = 800.0f;
static const float CLEAR_ANIM_TIME = 0.25f;
static const float IDLE_HINT_SEC = 20.0f;
static const float HINT_BLINK_PERIOD = 1.0f;

// Missile / wave timing
static const float WAVE_DELAY_PER_CELL = 0.06f;  // staggers missile launch by distance
static const float MISSILE_TRAVEL_TIME = 1.00f;  // flight duration
static const float MISSILE_ARC_HEIGHT = 36.0f;  // arc height

// Particles (FX)
static const int   BURST_PARTICLES = 60;
static const float BURST_SPEED_MIN = 220.0f;
static const float BURST_SPEED_MAX = 420.0f;
static const float BURST_LIFETIME = 0.80f;
static const float BURST_SIZE_MIN = 4.0f;
static const float BURST_SIZE_MAX = 24.0f;
static const float PARTICLE_GRAV_Y = 580.0f;
static const float PARTICLE_DRAG = 0.98f;

// Trail particles (for missiles)
static const int   TRAIL_PARTICLES_PER_STEP = 5;
static const float TRAIL_SPEED_MIN = 40.0f;
static const float TRAIL_SPEED_MAX = 90.0f;
static const float TRAIL_LIFETIME = 0.25f;
static const float TRAIL_SIZE_MIN = 1.5f;
static const float TRAIL_SIZE_MAX = 3.0f;

// Shake
static const float SHAKE_DURATION = 0.4f;
static const float SHAKE_MAG = 8.0f;
static const float SHAKE_FREQ = 42.0f;

// Cinematics
static const float SLOWMO_DURATION = 0.30f;
static const float SLOWMO_SCALE = 0.35f;
static const float FLASH_DURATION = 0.18f;

// Mid-wave mini beat. Small FX tunables for shakes/flashes.
static const float SECOND_SLOWMO_DURATION = 0.22f;
static const float SECOND_SHAKE_DURATION = 0.16f;
static const float SECOND_SHAKE_MAG = 6.0f;
static const float SECOND_FLASH_DURATION = 0.12f;

// Calculated offset to center the board, if you want to support resized windows later
// you need to make it a variable instead of a const
static const float offsetX = (SCREEN_WIDTH - BOARD_W) / 2.0f - PADDING;

// Core state enums and helpers shared by the game.
enum GameState 
{ 
    Idle, 
    Swapping, 
    Reverting, 
    Clearing, 
    FallingCollapse, 
    FallingRefill, 
    Shuffling 
};

enum SpecialType 
{ 
    SP_None = 0, 
    SP_CrossBomb = 1, 
    SP_ColorBomb = 2 
};

enum ClearMode 
{ 
    CM_Mask, 
    CM_Wave 
};

struct Vec2i 
{ 
    int r, c; 
};

// Convert board coords to pixel rect (for drawing a cell).
static Rectangle cellRect(int r, int c) 
{ 
    return Rectangle{ (float)PADDING + c * CELL + offsetX, (float)PADDING + r * CELL, (float)CELL, (float)CELL }; 
}

// Convert mouse pos to board coord (does not clamp; caller should InBounds()).
static Vec2i screenToCell(Vector2 m) 
{ 
    return Vec2i{ (int)((m.y - PADDING) / CELL), (int)((m.x - PADDING - offsetX) / CELL) }; 
}

// Basic ease helpers for animation curves.
static float EaseOutCubic(float t) 
{ 
    return 1.0f - powf(1.0f - t, 3.0f); 
}

static float EaseInOutCubic(float t) 
{ 
    return (t < 0.5f) ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2; 
}

static float clamp01(float x) 
{ 
    return x < 0 ? 0.f : (x > 1 ? 1.f : x); 
}

// Swap action state (cells a<->b with normalized time t in [0,1]).
struct Swap 
{
    Vec2i a; 
    Vec2i b; 
    float t;

    Swap() :a{ -1,-1 }, b{ -1,-1 }, t(0.f) {}
    
    void reset() 
    { 
        a = { -1,-1 }; 
        b = { -1,-1 }; 
        t = 0.f; 
    }
};

// Simple color palette mapping chip index -> display color.
static Color CHIP_COLORS[CHIP_TYPES] = 
{
    Color{  80, 180, 255, 255}, // blue
    Color{ 255, 105,  97, 255}, // red
    Color{ 255, 203,  64, 255}, // orange
    Color{ 120, 220, 100, 255}, // green
    Color{ 170, 120, 255, 255}, // purple
};

//End of Match3GameConfig.h
