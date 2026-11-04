#include "BonusGameWorld02.h"

void BonusGameWorld02::OnConfigureInput()
{
    Input.SetVirtualControlsVisible(true);
    InputAction attack = Input.RegisterAction("Attack");
    Input.BindKey(attack, KEY_SPACE);
    Input.AddTouchButton(attack, "ATTACK", TouchButtonAnchor::TopRight, 0);
}

