#include "BonusGameWorld01.h"

void BonusGameWorld01::OnConfigureInput()
{
    Input.SetVirtualControlsVisible(true);
    InputAction attack = Input.RegisterAction("Attack");
    Input.BindKey(attack, KEY_SPACE);
    Input.AddTouchButton(attack, "ATTACK", TouchButtonAnchor::TopRight, 0);

    InputAction lightUp = Input.RegisterAction("Light Up");
    Input.BindKey(lightUp, KEY_O);
    Input.AddTouchButton(lightUp, "LIGHT+", TouchButtonAnchor::TopRight, 1);

    InputAction lightDown = Input.RegisterAction("Light Down");
    Input.BindKey(lightDown, KEY_P);
    Input.AddTouchButton(lightDown, "LIGHT-", TouchButtonAnchor::TopRight, 2);
}

