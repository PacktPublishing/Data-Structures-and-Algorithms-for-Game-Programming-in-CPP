#pragma once

#include "InputSystem.h"

// Named input for this sample, registered in OnConfigureInput(). Gameplay code
// reads these instead of key codes, so the same code handles keyboard and
// touch input.
struct GameWorldInput
{
    InputAxis2D Move;               // WASD or arrows; left touch pad
    InputAxis2D LightDirection;     // J/K tilt x, I/M tilt z; right touch pad
    InputAction Attack;             // Space; ATTACK button
    InputAction LightUp;            // O; LIGHT+ button
    InputAction LightDown;          // P; LIGHT- button
    InputAction ToggleTerrainBounds;// B; keyboard only (debug)
};

extern GameWorldInput GameInput;
