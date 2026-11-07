#include "BonusGameWorld02.h"

GameWorldInput GameInput;

void BonusGameWorld02::OnConfigureInput()
{
	Input.SetControlScale(1.0f); // Set the scale for touch controls

    // Keyboard: WASD or the arrows. Touch: the left pad.
    GameInput.Move = Input.MoveAxis();
    Input.BindAxisTouchPad(GameInput.Move, TouchPad::Left);

    // Debug control for the shadow-casting light. The key layout keeps the
    // original bindings (J/K move x, I/M move z).
    GameInput.LightDirection = Input.RegisterAxis2D("Light Direction",
        KEY_I, KEY_M, KEY_K, KEY_J);
    Input.BindAxisTouchPad(GameInput.LightDirection, TouchPad::Right);

    GameInput.Attack = Input.RegisterAction("Attack");
    Input.BindKey(GameInput.Attack, KEY_SPACE);
    Input.AddTouchButton(GameInput.Attack, "ATTACK", TouchButtonAnchor::TopRight);

    GameInput.LightUp = Input.RegisterAction("Light Up");
    Input.BindKey(GameInput.LightUp, KEY_O);
    Input.AddTouchButton(GameInput.LightUp, "LIGHT+", TouchButtonAnchor::TopRight);

    GameInput.LightDown = Input.RegisterAction("Light Down");
    Input.BindKey(GameInput.LightDown, KEY_P);
    Input.AddTouchButton(GameInput.LightDown, "LIGHT-", TouchButtonAnchor::TopRight);

    GameInput.ToggleTerrainBounds = Input.RegisterAction("Toggle Terrain Bounds");
    Input.BindKey(GameInput.ToggleTerrainBounds, KEY_B);
}
