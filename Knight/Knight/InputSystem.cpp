#include "InputSystem.h"

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float DPadSize = 240.0f;
    constexpr float ControlMargin = 28.0f;
    constexpr float ButtonSize = 88.0f;
    constexpr float ButtonPadding = 20.0f;

    bool PointInRectangle(Vector2 point, Rectangle rectangle)
    {
        return CheckCollisionPointRec(point, rectangle);
    }
}

InputSystem::InputSystem()
{
#if defined(PLATFORM_ANDROID)
    profile = InputProfile::Touch;
#else
    profile = InputProfile::KeyboardMouse;
#endif
    virtualControlsVisible = profile == InputProfile::Touch;

    moveAxis = RegisterAxis2D("Move", KEY_W, KEY_S, KEY_A, KEY_D);
    secondaryAxis = RegisterAxis2D("Secondary", KEY_I, KEY_K, KEY_J, KEY_M);
}

void InputSystem::SetProfile(InputProfile newProfile)
{
    profile = newProfile;
    SetVirtualControlsVisible(profile == InputProfile::Touch);
    ResetPointers();
}

void InputSystem::Reset()
{
    actions.clear();
    axes.clear();
    buttons.clear();
    captures.clear();
    virtualKeys.clear();
    previousVirtualKeys.clear();
    ResetPointers();

    moveAxis = RegisterAxis2D("Move", KEY_W, KEY_S, KEY_A, KEY_D);
    secondaryAxis = RegisterAxis2D("Secondary", KEY_I, KEY_K, KEY_J, KEY_M);
}

void InputSystem::ResetPointers()
{
    primaryPointer = PointerState{};
    secondaryPointer = PointerState{};
    primaryTouchId = -1;
    lastPointerPosition = Vector2{ 0, 0 };
}

float InputSystem::WheelMove() const
{
    return profile == InputProfile::KeyboardMouse ? GetMouseWheelMove() : 0.0f;
}

InputAction InputSystem::RegisterAction(const char* name)
{
    for (int i = 0; i < static_cast<int>(actions.size()); ++i)
    {
        if (actions[i].name == name)
            return InputAction{ i };
    }

    actions.push_back(ActionDefinition{ name ? name : "", {} });
    return InputAction{ static_cast<int>(actions.size()) - 1 };
}

void InputSystem::BindKey(InputAction action, int key)
{
    if (!action.IsValid() || action.id >= static_cast<int>(actions.size()))
        return;

    std::vector<int>& keys = actions[action.id].keys;
    if (std::find(keys.begin(), keys.end(), key) == keys.end())
        keys.push_back(key);
}

void InputSystem::AddTouchButton(InputAction action, const char* label,
    TouchButtonAnchor anchor, int slot)
{
    if (!action.IsValid() || action.id >= static_cast<int>(actions.size()))
        return;

    for (TouchButton& button : buttons)
    {
        if (button.action.id == action.id)
        {
            button.label = label ? label : actions[action.id].name;
            button.anchor = anchor;
            button.slot = slot;
            return;
        }
    }

    TouchButton button;
    button.action = action;
    button.label = label ? label : actions[action.id].name;
    button.anchor = anchor;
    button.slot = slot;
    buttons.push_back(button);
}

InputAxis2D InputSystem::RegisterAxis2D(const char* name, int upKey,
    int downKey, int leftKey, int rightKey)
{
    for (int i = 0; i < static_cast<int>(axes.size()); ++i)
    {
        if (axes[i].name == name)
            return InputAxis2D{ i };
    }

    axes.push_back(AxisDefinition{
        name ? name : "", upKey, downKey, leftKey, rightKey });
    return InputAxis2D{ static_cast<int>(axes.size()) - 1 };
}

bool InputSystem::Down(InputAction action) const
{
    if (!action.IsValid() || action.id >= static_cast<int>(actions.size()))
        return false;

    for (int key : actions[action.id].keys)
    {
        if (IsKeyDown(key))
            return true;
    }
    return ActionVirtualDown(action);
}

bool InputSystem::Pressed(InputAction action) const
{
    if (!action.IsValid() || action.id >= static_cast<int>(actions.size()))
        return false;

    for (int key : actions[action.id].keys)
    {
        if (IsKeyPressed(key))
            return true;
    }

    for (const TouchButton& button : buttons)
        if (button.action.id == action.id && button.touchId >= 0 && button.previousTouchId < 0)
            return true;
    return false;
}

bool InputSystem::Released(InputAction action) const
{
    if (!action.IsValid() || action.id >= static_cast<int>(actions.size()))
        return false;

    for (int key : actions[action.id].keys)
    {
        if (IsKeyReleased(key))
            return true;
    }
    for (const TouchButton& button : buttons)
        if (button.action.id == action.id && button.touchId < 0 && button.previousTouchId >= 0)
            return true;
    return false;
}

Vector2 InputSystem::Axis(InputAxis2D axis) const
{
    if (!axis.IsValid() || axis.id >= static_cast<int>(axes.size()))
        return Vector2{ 0, 0 };

    const AxisDefinition& definition = axes[axis.id];
    Vector2 result = {
        (IsKeyDown(definition.rightKey) ? 1.0f : 0.0f) -
            (IsKeyDown(definition.leftKey) ? 1.0f : 0.0f),
        (IsKeyDown(definition.downKey) ? 1.0f : 0.0f) -
            (IsKeyDown(definition.upKey) ? 1.0f : 0.0f)
    };

    if (result.x != 0.0f && result.y != 0.0f)
    {
        constexpr float InverseSqrtTwo = 0.70710678f;
        result.x *= InverseSqrtTwo;
        result.y *= InverseSqrtTwo;
    }
    return result;
}

bool InputSystem::IsKeyDown(int key) const
{
    const auto it = virtualKeys.find(key);
    return ::IsKeyDown(key) || (it != virtualKeys.end() && it->second);
}

bool InputSystem::IsKeyPressed(int key) const
{
    const bool current = virtualKeys.count(key) > 0 && virtualKeys.at(key);
    const bool previous = previousVirtualKeys.count(key) > 0 && previousVirtualKeys.at(key);
    return ::IsKeyPressed(key) || (current && !previous);
}

bool InputSystem::IsKeyReleased(int key) const
{
    const bool current = virtualKeys.count(key) > 0 && virtualKeys.at(key);
    const bool previous = previousVirtualKeys.count(key) > 0 && previousVirtualKeys.at(key);
    return ::IsKeyReleased(key) || (!current && previous);
}

void InputSystem::SetVirtualControlsVisible(bool visible)
{
    virtualControlsVisible = visible && profile == InputProfile::Touch;
    if (!virtualControlsVisible)
    {
        captures.clear();
        virtualKeys.clear();
        for (TouchButton& button : buttons)
            button.touchId = -1;
    }
}

void InputSystem::BeginFrame()
{
    previousVirtualKeys = virtualKeys;
    virtualKeys.clear();
    for (TouchButton& button : buttons)
        button.previousTouchId = button.touchId;
    UpdateLayout();

    if (virtualControlsVisible)
        UpdateTouches();
    else
        captures.clear();

    UpdatePrimaryPointer();
    UpdateSecondaryPointer();
}

void InputSystem::UpdateSecondaryPointer()
{
    if (profile == InputProfile::Touch)
    {
        secondaryPointer = primaryPointer;
        return;
    }

    secondaryPointer.position = GetMousePosition();
    secondaryPointer.delta = GetMouseDelta();
    secondaryPointer.pressed = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    secondaryPointer.down = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    secondaryPointer.released = IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);
}

void InputSystem::UpdateLayout()
{
    const float width = static_cast<float>(GetScreenWidth());
    const float height = static_cast<float>(GetScreenHeight());

    leftDPadBounds = Rectangle{
        ControlMargin,
        height - DPadSize - ControlMargin,
        DPadSize,
        DPadSize
    };
    rightDPadBounds = Rectangle{
        width - DPadSize - ControlMargin,
        height - DPadSize - ControlMargin,
        DPadSize,
        DPadSize
    };

    for (TouchButton& button : buttons)
        button.bounds = ButtonBounds(button);
}

Rectangle InputSystem::ButtonBounds(const TouchButton& button) const
{
    const float width = static_cast<float>(GetScreenWidth());
    const float height = static_cast<float>(GetScreenHeight());
    const float stride = ButtonSize + ButtonPadding;
    const int column = button.slot % 3;
    const int row = button.slot / 3;

    float x = ControlMargin + column * stride;
    float y = ControlMargin + row * stride;

    if (button.anchor == TouchButtonAnchor::TopRight ||
        button.anchor == TouchButtonAnchor::BottomRight)
        x = width - ControlMargin - ButtonSize - column * stride;

    if (button.anchor == TouchButtonAnchor::BottomLeft ||
        button.anchor == TouchButtonAnchor::BottomRight)
        y = height - ControlMargin - ButtonSize - row * stride;

    return Rectangle{ x, y, ButtonSize, ButtonSize };
}

int InputSystem::FindTouchIndexById(int touchId) const
{
    const int count = GetTouchPointCount();
    for (int i = 0; i < count; ++i)
    {
        if (GetTouchPointId(i) == touchId)
            return i;
    }
    return -1;
}

bool InputSystem::IsTouchPresent(int touchId, Vector2* position) const
{
    const int index = FindTouchIndexById(touchId);
    if (index < 0)
        return false;
    if (position)
        *position = GetTouchPosition(index);
    return true;
}

bool InputSystem::IsTouchCaptured(int touchId) const
{
    for (const TouchCapture& capture : captures)
        if (capture.touchId == touchId)
            return true;
    return false;
}

void InputSystem::UpdatePrimaryPointer()
{
    primaryPointer.pressed = false;
    primaryPointer.released = false;
    primaryPointer.delta = Vector2{ 0, 0 };

    if (profile == InputProfile::KeyboardMouse)
    {
        primaryPointer.position = GetMousePosition();
        primaryPointer.delta = GetMouseDelta();
        primaryPointer.pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        primaryPointer.down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        primaryPointer.released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
        lastPointerPosition = primaryPointer.position;
        return;
    }

    Vector2 position;
    if (primaryTouchId >= 0 && IsTouchPresent(primaryTouchId, &position) &&
        !IsTouchCaptured(primaryTouchId))
    {
        primaryPointer.position = position;
        primaryPointer.delta = Vector2{
            position.x - lastPointerPosition.x,
            position.y - lastPointerPosition.y
        };
        primaryPointer.down = true;
        lastPointerPosition = position;
        return;
    }

    if (primaryTouchId >= 0)
    {
        primaryPointer.down = false;
        primaryPointer.released = true;
        primaryPointer.position = lastPointerPosition;
        primaryTouchId = -1;
        return;
    }

    const int touchCount = GetTouchPointCount();
    for (int i = 0; i < touchCount; ++i)
    {
        const int touchId = GetTouchPointId(i);
        if (IsTouchCaptured(touchId))
            continue;

        primaryTouchId = touchId;
        position = GetTouchPosition(i);
        primaryPointer.position = position;
        primaryPointer.down = true;
        primaryPointer.pressed = true;
        lastPointerPosition = position;
        return;
    }

    primaryPointer.down = false;
}

void InputSystem::UpdateTouches()
{
    // Update existing captures and discard fingers that were released.
    for (int i = static_cast<int>(captures.size()) - 1; i >= 0; --i)
    {
        Vector2 position;
        if (!IsTouchPresent(captures[i].touchId, &position))
        {
            if (captures[i].type == CapturedControlType::Button &&
                captures[i].index >= 0 && captures[i].index < static_cast<int>(buttons.size()))
                buttons[captures[i].index].touchId = -1;
            captures.erase(captures.begin() + i);
            continue;
        }

        if (captures[i].type == CapturedControlType::LeftDPad)
            UpdateDPad(position, true);
        else if (captures[i].type == CapturedControlType::RightDPad)
            UpdateDPad(position, false);
        else if (captures[i].index >= 0 && captures[i].index < static_cast<int>(buttons.size()))
        {
            const TouchButton& button = buttons[captures[i].index];
            for (int key : actions[button.action.id].keys)
                PressVirtualKey(key);
        }
    }

    // Capture newly-arrived fingers. A touch belongs to at most one control.
    const int touchCount = GetTouchPointCount();
    for (int touchIndex = 0; touchIndex < touchCount; ++touchIndex)
    {
        const int touchId = GetTouchPointId(touchIndex);
        bool alreadyCaptured = false;
        for (const TouchCapture& capture : captures)
            alreadyCaptured = alreadyCaptured || capture.touchId == touchId;
        if (alreadyCaptured)
            continue;

        const Vector2 position = GetTouchPosition(touchIndex);

        bool captured = false;
        for (int buttonIndex = 0; buttonIndex < static_cast<int>(buttons.size()); ++buttonIndex)
        {
            if (PointInRectangle(position, buttons[buttonIndex].bounds))
            {
                buttons[buttonIndex].touchId = touchId;
                captures.push_back(TouchCapture{
                    touchId, CapturedControlType::Button, buttonIndex });
                for (int key : actions[buttons[buttonIndex].action.id].keys)
                    PressVirtualKey(key);
                captured = true;
                break;
            }
        }
        if (captured)
            continue;

        if (PointInRectangle(position, leftDPadBounds))
        {
            captures.push_back(TouchCapture{
                touchId, CapturedControlType::LeftDPad, -1 });
            UpdateDPad(position, true);
        }
        else if (PointInRectangle(position, rightDPadBounds))
        {
            captures.push_back(TouchCapture{
                touchId, CapturedControlType::RightDPad, -1 });
            UpdateDPad(position, false);
        }
    }
}

void InputSystem::UpdateDPad(Vector2 position, bool leftPad)
{
    const Rectangle bounds = leftPad ? leftDPadBounds : rightDPadBounds;
    const Vector2 center = {
        bounds.x + bounds.width * 0.5f,
        bounds.y + bounds.height * 0.5f
    };
    const float dx = position.x - center.x;
    const float dy = position.y - center.y;
    const float deadZone = bounds.width * 0.14f;

    const float absX = std::fabs(dx);
    const float absY = std::fabs(dy);
    if (absX * absX + absY * absY < deadZone * deadZone)
        return;

    // Eight 45-degree sectors: a direction is active unless the finger is
    // more than 67.5 degrees away from it (tan(22.5) ~= 0.4142).
    constexpr float DiagonalRatio = 0.4142f;
    const bool horizontal = absX >= absY * DiagonalRatio;
    const bool vertical = absY >= absX * DiagonalRatio;

    if (horizontal)
    {
        if (leftPad)
        {
            PressVirtualKey(dx < 0 ? KEY_A : KEY_D);
            PressVirtualKey(dx < 0 ? KEY_LEFT : KEY_RIGHT);
        }
        else
            PressVirtualKey(dx < 0 ? KEY_J : KEY_M);
    }
    if (vertical)
    {
        if (leftPad)
        {
            PressVirtualKey(dy < 0 ? KEY_W : KEY_S);
            PressVirtualKey(dy < 0 ? KEY_UP : KEY_DOWN);
        }
        else
            PressVirtualKey(dy < 0 ? KEY_I : KEY_K);
    }
}

void InputSystem::PressVirtualKey(int key)
{
    virtualKeys[key] = true;
}

bool InputSystem::ActionVirtualDown(InputAction action) const
{
    for (const TouchButton& button : buttons)
    {
        if (button.action.id == action.id && button.touchId >= 0)
            return true;
    }
    return false;
}

void InputSystem::DrawOverlay() const
{
    if (!virtualControlsVisible)
        return;

    auto drawDPad = [](Rectangle bounds, const char* up, const char* left,
        const char* down, const char* right)
    {
        const float cell = bounds.width / 3.0f;
        const Color fill = Color{ 20, 24, 34, 145 };
        const Color outline = Color{ 230, 235, 255, 180 };
        const Color text = Color{ 255, 255, 255, 220 };
        const Rectangle cells[4] = {
            { bounds.x + cell, bounds.y, cell, cell },
            { bounds.x, bounds.y + cell, cell, cell },
            { bounds.x + cell, bounds.y + cell * 2.0f, cell, cell },
            { bounds.x + cell * 2.0f, bounds.y + cell, cell, cell }
        };
        const char* labels[4] = { up, left, down, right };

        for (int i = 0; i < 4; ++i)
        {
            DrawRectangleRounded(cells[i], 0.25f, 6, fill);
            DrawRectangleRoundedLines(cells[i], 0.25f, 6, outline);
            const int fontSize = static_cast<int>(cell * 0.38f);
            const int textWidth = MeasureText(labels[i], fontSize);
            DrawText(labels[i],
                static_cast<int>(cells[i].x + (cell - textWidth) * 0.5f),
                static_cast<int>(cells[i].y + (cell - fontSize) * 0.5f),
                fontSize, text);
        }
    };

    drawDPad(leftDPadBounds, "W", "A", "S", "D");
    drawDPad(rightDPadBounds, "I", "J", "K", "M");

    for (const TouchButton& button : buttons)
    {
        const bool down = button.touchId >= 0;
        const Color fill = down ? Color{ 70, 130, 240, 205 } : Color{ 20, 24, 34, 165 };
        const Color outline = Color{ 235, 240, 255, 210 };
        DrawCircle(static_cast<int>(button.bounds.x + button.bounds.width * 0.5f),
            static_cast<int>(button.bounds.y + button.bounds.height * 0.5f),
            button.bounds.width * 0.5f, fill);
        DrawCircleLines(static_cast<int>(button.bounds.x + button.bounds.width * 0.5f),
            static_cast<int>(button.bounds.y + button.bounds.height * 0.5f),
            button.bounds.width * 0.5f, outline);
        const int fontSize = 22;
        const int textWidth = MeasureText(button.label.c_str(), fontSize);
        DrawText(button.label.c_str(),
            static_cast<int>(button.bounds.x + (button.bounds.width - textWidth) * 0.5f),
            static_cast<int>(button.bounds.y + (button.bounds.height - fontSize) * 0.5f),
            fontSize, WHITE);
    }
}

