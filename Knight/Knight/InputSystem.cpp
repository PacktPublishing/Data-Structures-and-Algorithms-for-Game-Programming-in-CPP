#include "InputSystem.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <utility>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

namespace
{
    // Control sizes in density-independent pixels (1 dp = 1 px at 160 dpi),
    // so controls have the same physical size on every screen.
    constexpr float PadSizeDp = 130.0f;
    constexpr float ControlMarginDp = 16.0f;
    constexpr float ButtonSizeDp = 56.0f;
    constexpr float ButtonPaddingDp = 12.0f;
    // On small or low-density screens a pad never takes more than this share
    // of the screen's short side; every other control shrinks with it.
    constexpr float MaxPadShare = 0.5f;
    // Dead zone at the center of a pad, as a fraction of the pad's size.
    constexpr float PadDeadZone = 0.14f;
    // Floating pads can be grabbed anywhere below this fraction of the height.
    constexpr float FloatingZoneTop = 0.35f;

    constexpr double LongPressSeconds = 0.5;
    constexpr float LongPressSlopDp = 12.0f;
    // Spreading two fingers by the screen's short side zooms this many wheel
    // notches.
    constexpr float PinchNotchesPerShortSide = 10.0f;

    bool PointInRectangle(Vector2 point, Rectangle rectangle)
    {
        return CheckCollisionPointRec(point, rectangle);
    }

    float Distance(Vector2 a, Vector2 b)
    {
        const float dx = b.x - a.x;
        const float dy = b.y - a.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    RaylibInputBackend raylibBackend;

    // Short on-screen label for a key code; letters and digits only.
    std::string KeyLabel(int key)
    {
        if ((key >= KEY_A && key <= KEY_Z) || (key >= KEY_ZERO && key <= KEY_NINE))
            return std::string(1, static_cast<char>(key));
        return std::string();
    }

    // Default caption for a button that stands in for a key.
    std::string KeyButtonLabel(int key)
    {
        const std::string letter = KeyLabel(key);
        if (!letter.empty())
            return letter;
        if (key >= KEY_F1 && key <= KEY_F12)
            return "F" + std::to_string(key - KEY_F1 + 1);
        switch (key)
        {
        case KEY_SPACE: return "SPACE";
        case KEY_ENTER: return "ENTER";
        case KEY_BACKSPACE: return "BACK";
        case KEY_TAB: return "TAB";
        case KEY_UP: return "UP";
        case KEY_DOWN: return "DOWN";
        case KEY_LEFT: return "LEFT";
        case KEY_RIGHT: return "RIGHT";
        default: return "KEY " + std::to_string(key);
        }
    }
}

InputSystem::InputSystem()
    : backend(&raylibBackend)
{
#if defined(PLATFORM_ANDROID)
    profile = InputProfile::Touch;
#else
    profile = InputProfile::KeyboardMouse;
#if defined(PLATFORM_WEB)
    // Phones and tablets (a coarse primary pointer) get the touch controls.
    // The page's URL can force it either way: ?touch or ?touch=0.
    if (emscripten_run_script_int(
            "(function () {"
            "  var q = new URLSearchParams(window.location.search);"
            "  if (q.has('touch')) return q.get('touch') !== '0' ? 1 : 0;"
            "  return window.matchMedia('(pointer: coarse)').matches ? 1 : 0;"
            "})()"))
        profile = InputProfile::Touch;
#endif
    // Set KNIGHT_INPUT_PROFILE=touch to preview the Android controls on
    // desktop with the mouse standing in for one finger.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996) // getenv is fine for a read-only lookup
#endif
    const char* forcedProfile = std::getenv("KNIGHT_INPUT_PROFILE");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
    if (forcedProfile && std::strcmp(forcedProfile, "touch") == 0)
        profile = InputProfile::Touch;
#endif
    virtualControlsVisible = profile == InputProfile::Touch;

    RegisterDefaults();
}

void InputSystem::RegisterDefaults()
{
    // Keys only: a sample that wants a pad binds one explicitly, so samples
    // that never read these axes show no pads.
    moveAxis = RegisterAxis2D("Move", KEY_W, KEY_S, KEY_A, KEY_D);
    BindAxisKeys(moveAxis, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);

    secondaryAxis = RegisterAxis2D("Secondary", KEY_I, KEY_K, KEY_J, KEY_M);

    backAction = RegisterAction("Back");
    BindKey(backAction, KEY_BACK);
}

void InputSystem::SetBackend(const InputBackend* newBackend)
{
    backend = newBackend ? newBackend : &raylibBackend;
    ResetTouchState();
}

float InputSystem::ShortSide() const
{
    return static_cast<float>(std::min(backend->ScreenWidth(), backend->ScreenHeight()));
}

void InputSystem::SetProfile(InputProfile newProfile)
{
    profile = newProfile;
    SetVirtualControlsVisible(profile == InputProfile::Touch);
    ResetTouchState();
}

void InputSystem::SetControlScale(float scale)
{
    controlScale = std::max(scale, MinControlScale);
}

void InputSystem::Reset()
{
    actions.clear();
    axes.clear();
    buttons.clear();
    actionDown.clear();
    previousActionDown.clear();
    for (PadState& pad : pads)
        pad = PadState{};
    virtualKeys.clear();
    previousVirtualKeys.clear();
    ResetTouchState();

    RegisterDefaults();
}

void InputSystem::ResetTouchState()
{
    captures.clear();
    for (TouchButton& button : buttons)
        button.touchId = -1;
    for (PadState& pad : pads)
    {
        pad.touchId = -1;
        pad.value = Vector2{ 0, 0 };
    }

    primaryPointer = PointerState{};
    secondaryPointer = PointerState{};
    primaryTouchId = -1;
    lastPointerPosition = Vector2{ 0, 0 };

    zoomDelta = 0.0f;
    panDelta = Vector2{ 0, 0 };
    pinchTouchIds[0] = pinchTouchIds[1] = -1;
    waitForAllFingersUp = false;
    longPressCancelled = false;
    longPressFired = false;
}

float InputSystem::WheelMove() const
{
    return profile == InputProfile::KeyboardMouse ? backend->MouseWheelMove() : 0.0f;
}

InputAction InputSystem::RegisterAction(const char* name)
{
    for (int i = 0; i < static_cast<int>(actions.size()); ++i)
    {
        if (actions[i].name == name)
            return InputAction{ i };
    }

    actions.push_back(ActionDefinition{ name ? name : "", {} });
    actionDown.push_back(false);
    previousActionDown.push_back(false);
    return InputAction{ static_cast<int>(actions.size()) - 1 };
}

InputAction InputSystem::FindAction(const char* name) const
{
    for (int i = 0; i < static_cast<int>(actions.size()); ++i)
    {
        if (actions[i].name == name)
            return InputAction{ i };
    }
    return InputAction{};
}

bool InputSystem::IsValidAction(InputAction action) const
{
    return action.IsValid() && action.id < static_cast<int>(actions.size());
}

void InputSystem::BindKey(InputAction action, int key)
{
    if (!IsValidAction(action))
        return;

    std::vector<int>& keys = actions[action.id].keys;
    if (std::find(keys.begin(), keys.end(), key) == keys.end())
        keys.push_back(key);
}

void InputSystem::AddTouchButton(InputAction action, const char* label,
    TouchButtonAnchor anchor, int slot)
{
    if (!IsValidAction(action))
        return;

    if (slot < 0)
        slot = NextFreeSlot(anchor, action.id);

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

int InputSystem::NextFreeSlot(TouchButtonAnchor anchor, int ignoredActionId) const
{
    for (int slot = 0;; ++slot)
    {
        bool taken = false;
        for (const TouchButton& button : buttons)
        {
            if (button.action.id != ignoredActionId && button.anchor == anchor &&
                button.slot == slot)
            {
                taken = true;
                break;
            }
        }
        if (!taken)
            return slot;
    }
}

InputAction InputSystem::AddKeyButton(int key, const char* label,
    TouchButtonAnchor anchor, int slot)
{
    const std::string caption = label ? std::string(label) : KeyButtonLabel(key);
    const InputAction action = RegisterAction(("Key " + caption).c_str());
    BindKey(action, key);
    AddTouchButton(action, caption.c_str(), anchor, slot);
    return action;
}

InputAxis2D InputSystem::AddKeyPad(TouchPad pad, const char* name, int upKey,
    int downKey, int leftKey, int rightKey, bool floating)
{
    const InputAxis2D axis = RegisterAxis2D(name, upKey, downKey, leftKey, rightKey);
    if (pad == TouchPad::None)
        return axis;

    // A pad drives one axis; detach the previous owner so its keys are not
    // pressed too.
    for (AxisDefinition& definition : axes)
    {
        if (definition.pad == pad)
            definition.pad = TouchPad::None;
    }
    BindAxisTouchPad(axis, pad);
    SetTouchPadFloating(pad, floating);
    return axis;
}

void InputSystem::ClearVirtualControls()
{
    buttons.clear();
    for (AxisDefinition& definition : axes)
        definition.pad = TouchPad::None;
    for (PadState& pad : pads)
        pad = PadState{};
    virtualKeys.clear();
    previousVirtualKeys.clear();
    ResetTouchState();
}

InputAxis2D InputSystem::RegisterAxis2D(const char* name, int upKey,
    int downKey, int leftKey, int rightKey)
{
    for (int i = 0; i < static_cast<int>(axes.size()); ++i)
    {
        if (axes[i].name == name)
            return InputAxis2D{ i };
    }

    AxisDefinition definition;
    definition.name = name ? name : "";
    definition.keySets.push_back(AxisKeys{ upKey, downKey, leftKey, rightKey });
    axes.push_back(definition);
    return InputAxis2D{ static_cast<int>(axes.size()) - 1 };
}

void InputSystem::BindAxisKeys(InputAxis2D axis, int upKey, int downKey,
    int leftKey, int rightKey)
{
    if (!axis.IsValid() || axis.id >= static_cast<int>(axes.size()))
        return;
    axes[axis.id].keySets.push_back(AxisKeys{ upKey, downKey, leftKey, rightKey });
}

void InputSystem::BindAxisTouchPad(InputAxis2D axis, TouchPad pad)
{
    if (!axis.IsValid() || axis.id >= static_cast<int>(axes.size()))
        return;
    axes[axis.id].pad = pad;
}

void InputSystem::SetTouchPadFloating(TouchPad pad, bool floating)
{
    if (pad == TouchPad::None)
        return;
    pads[static_cast<int>(pad)].floating = floating;
}

bool InputSystem::Down(InputAction action) const
{
    return IsValidAction(action) && actionDown[action.id];
}

bool InputSystem::Pressed(InputAction action) const
{
    if (!IsValidAction(action))
        return false;
    if (actionDown[action.id] && !previousActionDown[action.id])
        return true;

    // A key tapped and released between two frames never reads as held.
    for (int key : actions[action.id].keys)
    {
        if (backend->IsKeyPressed(key))
            return true;
    }
    return false;
}

bool InputSystem::Released(InputAction action) const
{
    if (!IsValidAction(action))
        return false;
    if (!actionDown[action.id] && previousActionDown[action.id])
        return true;

    for (int key : actions[action.id].keys)
    {
        if (backend->IsKeyReleased(key))
            return true;
    }
    return false;
}

Vector2 InputSystem::Axis(InputAxis2D axis) const
{
    if (!axis.IsValid() || axis.id >= static_cast<int>(axes.size()))
        return Vector2{ 0, 0 };

    const AxisDefinition& definition = axes[axis.id];
    bool up = false, down = false, left = false, right = false;
    for (const AxisKeys& keys : definition.keySets)
    {
        up = up || backend->IsKeyDown(keys.upKey);
        down = down || backend->IsKeyDown(keys.downKey);
        left = left || backend->IsKeyDown(keys.leftKey);
        right = right || backend->IsKeyDown(keys.rightKey);
    }

    const Vector2 padValue = PadValue(definition.pad);
    const float keyX = (right ? 1.0f : 0.0f) - (left ? 1.0f : 0.0f);
    const float keyY = (down ? 1.0f : 0.0f) - (up ? 1.0f : 0.0f);
    return Vector2{
        keyX != 0.0f ? keyX : padValue.x,
        keyY != 0.0f ? keyY : padValue.y
    };
}

Vector2 InputSystem::PadValue(TouchPad pad) const
{
    return pads[static_cast<int>(pad)].value;
}

bool InputSystem::IsKeyDown(int key) const
{
    const auto it = virtualKeys.find(key);
    return backend->IsKeyDown(key) || (it != virtualKeys.end() && it->second);
}

bool InputSystem::IsKeyPressed(int key) const
{
    const bool current = virtualKeys.count(key) > 0 && virtualKeys.at(key);
    const bool previous = previousVirtualKeys.count(key) > 0 && previousVirtualKeys.at(key);
    return backend->IsKeyPressed(key) || (current && !previous);
}

bool InputSystem::IsKeyReleased(int key) const
{
    const bool current = virtualKeys.count(key) > 0 && virtualKeys.at(key);
    const bool previous = previousVirtualKeys.count(key) > 0 && previousVirtualKeys.at(key);
    return backend->IsKeyReleased(key) || (!current && previous);
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
        for (PadState& pad : pads)
            pad.touchId = -1;
    }
}

void InputSystem::BeginFrame()
{
    for (TouchButton& button : buttons)
        button.previousTouchId = button.touchId;
    for (PadState& pad : pads)
        pad.value = Vector2{ 0, 0 };
    zoomDelta = 0.0f;
    panDelta = Vector2{ 0, 0 };

    // Fingers that were down when the app lost focus (e.g. the user switched
    // apps) never report a release, so start over whenever focus changes.
    const bool focused = backend->IsWindowFocused();
    if (focused != wasFocused)
    {
        ResetTouchState();
        wasFocused = focused;
    }

    UpdateLayout();

    if (virtualControlsVisible && focused)
        UpdateTouches();
    else
        captures.clear();

    UpdateActions();
    UpdateVirtualKeys();

    if (profile == InputProfile::KeyboardMouse)
    {
        UpdateMousePointers();
    }
    else
    {
        UpdateTouchPointers();
        // When touch is emulated with a mouse, the wheel stands in for a
        // pinch. Touch devices report no wheel, so this changes nothing there.
        if (zoomDelta == 0.0f)
            zoomDelta = backend->MouseWheelMove();
    }
}

void InputSystem::UpdateActions()
{
    previousActionDown = actionDown;
    for (int i = 0; i < static_cast<int>(actions.size()); ++i)
    {
        bool down = false;
        for (int key : actions[i].keys)
            down = down || backend->IsKeyDown(key);
        for (const TouchButton& button : buttons)
            down = down || (button.action.id == i && button.touchId >= 0);
        actionDown[i] = down;
    }
}

void InputSystem::UpdateVirtualKeys()
{
    previousVirtualKeys = virtualKeys;
    virtualKeys.clear();

    for (const TouchButton& button : buttons)
    {
        if (button.touchId < 0)
            continue;
        for (int key : actions[button.action.id].keys)
            if (key != KEY_NULL)
                virtualKeys[key] = true;
    }

    // A pad presses the direction keys of every key set on its axis, using
    // eight 45-degree sectors: a direction counts unless the finger is more
    // than 67.5 degrees away from it (tan(22.5) ~= 0.4142).
    constexpr float DiagonalRatio = 0.4142f;
    for (const AxisDefinition& axis : axes)
    {
        const Vector2 value = PadValue(axis.pad);
        const float absX = std::fabs(value.x);
        const float absY = std::fabs(value.y);
        if (absX == 0.0f && absY == 0.0f)
            continue;

        const bool horizontal = absX >= absY * DiagonalRatio;
        const bool vertical = absY >= absX * DiagonalRatio;
        for (const AxisKeys& keys : axis.keySets)
        {
            if (horizontal)
                virtualKeys[value.x < 0 ? keys.leftKey : keys.rightKey] = true;
            if (vertical)
                virtualKeys[value.y < 0 ? keys.upKey : keys.downKey] = true;
        }
        // Directions a pad leaves unbound (KEY_NULL) press nothing.
        virtualKeys.erase(KEY_NULL);
    }
}

void InputSystem::UpdateMousePointers()
{
    primaryPointer.position = backend->MousePosition();
    primaryPointer.delta = backend->MouseDelta();
    primaryPointer.pressed = backend->IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    primaryPointer.down = backend->IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    primaryPointer.released = backend->IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    lastPointerPosition = primaryPointer.position;

    secondaryPointer.position = primaryPointer.position;
    secondaryPointer.delta = primaryPointer.delta;
    secondaryPointer.pressed = backend->IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
    secondaryPointer.down = backend->IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    secondaryPointer.released = backend->IsMouseButtonReleased(MOUSE_BUTTON_RIGHT);

    zoomDelta = backend->MouseWheelMove();
}

void InputSystem::UpdateLayout()
{
    padSize = std::min(PadSizeDp * controlScale * backend->DpScale(),
        ShortSide() * MaxPadShare);
    const float scale = padSize / PadSizeDp;
    buttonSize = ButtonSizeDp * scale;
    buttonPadding = ButtonPaddingDp * scale;
    controlMargin = ControlMarginDp * scale;

    for (TouchButton& button : buttons)
        button.bounds = ButtonBounds(button);

    // Idle pads rest in their corner; a held floating pad keeps the center
    // set when it was grabbed.
    for (TouchPad pad : { TouchPad::Left, TouchPad::Right })
    {
        PadState& state = pads[static_cast<int>(pad)];
        if (state.touchId < 0)
        {
            const Rectangle bounds = PadBounds(pad);
            state.center = Vector2{
                bounds.x + bounds.width * 0.5f,
                bounds.y + bounds.height * 0.5f
            };
        }
    }
}

Rectangle InputSystem::ButtonBounds(const TouchButton& button) const
{
    const float width = static_cast<float>(backend->ScreenWidth());
    const float height = static_cast<float>(backend->ScreenHeight());
    const float stride = buttonSize + buttonPadding;
    const int column = button.slot % 3;
    const int row = button.slot / 3;

    float x = controlMargin + column * stride;
    float y = controlMargin + row * stride;

    if (button.anchor == TouchButtonAnchor::TopRight ||
        button.anchor == TouchButtonAnchor::BottomRight)
        x = width - controlMargin - buttonSize - column * stride;

    if (button.anchor == TouchButtonAnchor::BottomLeft ||
        button.anchor == TouchButtonAnchor::BottomRight)
        y = height - controlMargin - buttonSize - row * stride;

    return Rectangle{ x, y, buttonSize, buttonSize };
}

Rectangle InputSystem::PadBounds(TouchPad pad) const
{
    const float width = static_cast<float>(backend->ScreenWidth());
    const float height = static_cast<float>(backend->ScreenHeight());
    const float x = pad == TouchPad::Left
        ? controlMargin
        : width - padSize - controlMargin;
    return Rectangle{ x, height - padSize - controlMargin, padSize, padSize };
}

Rectangle InputSystem::PadCaptureZone(TouchPad pad) const
{
    if (!pads[static_cast<int>(pad)].floating)
        return PadBounds(pad);

    const float width = static_cast<float>(backend->ScreenWidth());
    const float height = static_cast<float>(backend->ScreenHeight());
    const float top = height * FloatingZoneTop;
    const float halfWidth = width * 0.5f;
    return Rectangle{
        pad == TouchPad::Left ? 0.0f : halfWidth, top,
        halfWidth, height - top
    };
}

const InputSystem::AxisDefinition* InputSystem::AxisForPad(TouchPad pad) const
{
    for (const AxisDefinition& axis : axes)
    {
        if (axis.pad == pad)
            return &axis;
    }
    return nullptr;
}

int InputSystem::FindTouchIndexById(int touchId) const
{
    const int count = backend->TouchCount();
    for (int i = 0; i < count; ++i)
    {
        if (backend->TouchId(i) == touchId)
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
        *position = backend->TouchPosition(index);
    return true;
}

bool InputSystem::IsTouchCaptured(int touchId) const
{
    for (const TouchCapture& capture : captures)
        if (capture.touchId == touchId)
            return true;
    return false;
}

void InputSystem::UpdateTouchPointers()
{
    primaryPointer.pressed = false;
    primaryPointer.released = false;
    primaryPointer.delta = Vector2{ 0, 0 };
    secondaryPointer.pressed = false;
    secondaryPointer.released = false;

    // Fingers not held by an on-screen control.
    std::vector<FreeTouch> freeTouches;
    const int touchCount = backend->TouchCount();
    for (int i = 0; i < touchCount; ++i)
    {
        const int touchId = backend->TouchId(i);
        if (!IsTouchCaptured(touchId))
            freeTouches.push_back(FreeTouch{ touchId, backend->TouchPosition(i) });
    }

    if (freeTouches.size() >= 2)
    {
        // A second finger turns the gesture into a pinch: end the one-finger
        // drag so cameras stop orbiting, and keep it ended until every finger
        // has lifted, so the last finger of a pinch does not start a drag.
        if (primaryTouchId >= 0)
            ReleasePrimaryTouch();
        waitForAllFingersUp = true;
        UpdatePinch(freeTouches);
    }
    else
    {
        pinchTouchIds[0] = pinchTouchIds[1] = -1;
        if (freeTouches.empty())
            waitForAllFingersUp = false;

        const FreeTouch* primary = nullptr;
        for (const FreeTouch& touch : freeTouches)
            if (touch.id == primaryTouchId)
                primary = &touch;

        if (primary)
        {
            primaryPointer.position = primary->position;
            primaryPointer.delta = Vector2{
                primary->position.x - lastPointerPosition.x,
                primary->position.y - lastPointerPosition.y
            };
            primaryPointer.down = true;
            lastPointerPosition = primary->position;
        }
        else if (primaryTouchId >= 0)
        {
            ReleasePrimaryTouch();
        }
        else if (!freeTouches.empty() && !waitForAllFingersUp)
        {
            const FreeTouch& touch = freeTouches.front();
            primaryTouchId = touch.id;
            primaryPointer.position = touch.position;
            primaryPointer.down = true;
            primaryPointer.pressed = true;
            lastPointerPosition = touch.position;

            primaryPressTime = backend->Time();
            primaryPressPosition = touch.position;
            longPressCancelled = false;
            longPressFired = false;
        }
        else
        {
            primaryPointer.down = false;
        }
    }

    UpdateLongPress();

    secondaryPointer.position = primaryPointer.position;
    secondaryPointer.delta = primaryPointer.delta;
    secondaryPointer.down = primaryPointer.down;
}

void InputSystem::ReleasePrimaryTouch()
{
    primaryPointer.down = false;
    primaryPointer.released = true;
    primaryPointer.position = lastPointerPosition;
    primaryTouchId = -1;

    if (longPressFired)
        secondaryPointer.released = true;
    longPressFired = false;
}

void InputSystem::UpdateLongPress()
{
    if (!primaryPointer.down || longPressFired || longPressCancelled)
        return;

    // Moving the finger turns the press into a drag for good.
    if (Distance(primaryPressPosition, primaryPointer.position) > LongPressSlopDp * backend->DpScale())
    {
        longPressCancelled = true;
        return;
    }

    if (backend->Time() - primaryPressTime >= LongPressSeconds)
    {
        longPressFired = true;
        secondaryPointer.pressed = true;
    }
}

void InputSystem::UpdatePinch(const std::vector<FreeTouch>& freeTouches)
{
    const FreeTouch* first = nullptr;
    const FreeTouch* second = nullptr;
    for (const FreeTouch& touch : freeTouches)
    {
        if (touch.id == pinchTouchIds[0])
            first = &touch;
        else if (touch.id == pinchTouchIds[1])
            second = &touch;
    }

    const bool newPair = !first || !second;
    if (newPair)
    {
        first = &freeTouches[0];
        second = &freeTouches[1];
        pinchTouchIds[0] = first->id;
        pinchTouchIds[1] = second->id;
    }

    const float distance = Distance(first->position, second->position);
    const Vector2 midpoint = {
        (first->position.x + second->position.x) * 0.5f,
        (first->position.y + second->position.y) * 0.5f
    };

    // The first frame of a pair only records where the fingers are.
    if (!newPair)
    {
        zoomDelta = (distance - lastPinchDistance) * PinchNotchesPerShortSide / ShortSide();
        panDelta = Vector2{
            midpoint.x - lastPinchMidpoint.x,
            midpoint.y - lastPinchMidpoint.y
        };
    }
    lastPinchDistance = distance;
    lastPinchMidpoint = midpoint;
}

void InputSystem::UpdateTouches()
{
    // Update existing captures and discard fingers that were released.
    for (int i = static_cast<int>(captures.size()) - 1; i >= 0; --i)
    {
        const TouchCapture& capture = captures[i];
        Vector2 position;
        if (!IsTouchPresent(capture.touchId, &position))
        {
            if (capture.type == CapturedControlType::Button &&
                capture.index >= 0 && capture.index < static_cast<int>(buttons.size()))
                buttons[capture.index].touchId = -1;
            else if (capture.type == CapturedControlType::Pad)
                pads[capture.index].touchId = -1;
            captures.erase(captures.begin() + i);
            continue;
        }

        if (capture.type == CapturedControlType::Pad)
            UpdatePad(position, static_cast<TouchPad>(capture.index));
    }

    // Capture newly-arrived fingers. A touch belongs to at most one control,
    // and buttons win over a floating pad's zone.
    const int touchCount = backend->TouchCount();
    for (int touchIndex = 0; touchIndex < touchCount; ++touchIndex)
    {
        const int touchId = backend->TouchId(touchIndex);
        if (IsTouchCaptured(touchId))
            continue;

        // A finger that is already dragging the view does not grab a control
        // it slides onto.
        if (touchId == primaryTouchId ||
            touchId == pinchTouchIds[0] || touchId == pinchTouchIds[1])
            continue;

        const Vector2 position = backend->TouchPosition(touchIndex);

        bool captured = false;
        for (int buttonIndex = 0; buttonIndex < static_cast<int>(buttons.size()); ++buttonIndex)
        {
            if (PointInRectangle(position, buttons[buttonIndex].bounds))
            {
                buttons[buttonIndex].touchId = touchId;
                captures.push_back(TouchCapture{
                    touchId, CapturedControlType::Button, buttonIndex });
                captured = true;
                break;
            }
        }
        if (captured)
            continue;

        for (TouchPad pad : { TouchPad::Left, TouchPad::Right })
        {
            PadState& state = pads[static_cast<int>(pad)];
            if (state.touchId >= 0 || !AxisForPad(pad) ||
                !PointInRectangle(position, PadCaptureZone(pad)))
                continue;

            state.touchId = touchId;
            if (state.floating)
                state.center = position;
            captures.push_back(TouchCapture{
                touchId, CapturedControlType::Pad, static_cast<int>(pad) });
            UpdatePad(position, pad);
            break;
        }
    }
}

void InputSystem::UpdatePad(Vector2 position, TouchPad pad)
{
    PadState& state = pads[static_cast<int>(pad)];
    const float radius = padSize * 0.5f;
    float dx = position.x - state.center.x;
    float dy = position.y - state.center.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // A floating pad's base trails a thumb that slides past its edge, so
    // reversing direction responds immediately.
    if (state.floating && distance > radius)
    {
        const float excess = distance - radius;
        state.center.x += dx / distance * excess;
        state.center.y += dy / distance * excess;
        dx = position.x - state.center.x;
        dy = position.y - state.center.y;
        distance = radius;
    }

    const float deadZone = padSize * PadDeadZone;
    if (distance < deadZone)
        return;

    // Rescale so the value starts at 0 at the dead zone's edge and reaches 1
    // at the pad's edge; fingers beyond the edge stay at full strength.
    const float strength = std::min((distance - deadZone) / (radius - deadZone), 1.0f);
    state.value = Vector2{
        dx / distance * strength,
        dy / distance * strength
    };
}

void InputSystem::DrawOverlay() const
{
    if (!virtualControlsVisible)
        return;

    const Color baseFill = Color{ 20, 24, 34, 120 };
    const Color outline = Color{ 230, 235, 255, 170 };
    const Color labelColor = Color{ 255, 255, 255, 190 };
    const Color knobIdle = Color{ 230, 235, 255, 150 };
    const Color knobActive = Color{ 70, 130, 240, 220 };

    for (TouchPad pad : { TouchPad::Left, TouchPad::Right })
    {
        const AxisDefinition* axis = AxisForPad(pad);
        if (!axis || axis->keySets.empty())
            continue;

        const PadState& state = pads[static_cast<int>(pad)];
        const bool active = state.touchId >= 0;
        const float radius = padSize * 0.5f;
        const Vector2 center = state.center;

        DrawCircleV(center, radius, baseFill);
        DrawCircleLinesV(center, radius, outline);

        // Key hints at the four edges, so keyboard users of the same sample
        // recognize the controls.
        const AxisKeys& keys = axis->keySets.front();
        const int fontSize = static_cast<int>(radius * 0.24f);
        const float labelOffset = radius * 0.72f;
        const struct { int key; float x; float y; } labels[4] = {
            { keys.upKey, 0.0f, -labelOffset },
            { keys.leftKey, -labelOffset, 0.0f },
            { keys.downKey, 0.0f, labelOffset },
            { keys.rightKey, labelOffset, 0.0f }
        };
        for (const auto& label : labels)
        {
            const std::string text = KeyLabel(label.key);
            if (text.empty())
                continue;
            const int textWidth = MeasureText(text.c_str(), fontSize);
            DrawText(text.c_str(),
                static_cast<int>(center.x + label.x - textWidth * 0.5f),
                static_cast<int>(center.y + label.y - fontSize * 0.5f),
                fontSize, labelColor);
        }

        DrawCircleV(Vector2{
                center.x + state.value.x * radius,
                center.y + state.value.y * radius },
            radius * 0.38f, active ? knobActive : knobIdle);
    }

    for (const TouchButton& button : buttons)
    {
        const bool down = button.touchId >= 0;
        const Color fill = down ? Color{ 70, 130, 240, 205 } : Color{ 20, 24, 34, 165 };
        const Color buttonOutline = Color{ 235, 240, 255, 210 };
        const Vector2 center = {
            button.bounds.x + button.bounds.width * 0.5f,
            button.bounds.y + button.bounds.height * 0.5f
        };
        DrawCircleV(center, button.bounds.width * 0.5f, fill);
        DrawCircleLinesV(center, button.bounds.width * 0.5f, buttonOutline);
        const int fontSize = std::max(10, static_cast<int>(button.bounds.width * 0.25f));
        const int textWidth = MeasureText(button.label.c_str(), fontSize);
        DrawText(button.label.c_str(),
            static_cast<int>(center.x - textWidth * 0.5f),
            static_cast<int>(center.y - fontSize * 0.5f),
            fontSize, WHITE);
    }
}

void InputSystem::DrawDebugOverlay() const
{
    const int fontSize = std::max(14, backend->ScreenHeight() / 48);
    const int lineHeight = fontSize + 4;
    const Color idle = Color{ 200, 205, 215, 255 };
    const Color active = Color{ 120, 230, 120, 255 };

    struct Line
    {
        std::string name;
        std::string value;
        bool highlighted;
    };
    std::vector<Line> lines;
    auto add = [&lines](const std::string& name, const std::string& value,
        bool highlighted)
    {
        lines.push_back(Line{ name, value, highlighted });
    };

    add(profile == InputProfile::Touch ? "Touch profile" : "KeyboardMouse profile",
        TextFormat("%d touch(es)", backend->TouchCount()), false);

    for (int i = 0; i < static_cast<int>(actions.size()); ++i)
    {
        const InputAction action{ i };
        add(actions[i].name, TextFormat("%s%s", Down(action) ? "DOWN" : "-",
            Pressed(action) ? " pressed" : ""), Down(action));
    }

    for (int i = 0; i < static_cast<int>(axes.size()); ++i)
    {
        const Vector2 value = Axis(InputAxis2D{ i });
        add(axes[i].name, TextFormat("(%+.2f, %+.2f)", value.x, value.y),
            value.x != 0.0f || value.y != 0.0f);
    }

    auto pointerValue = [](const PointerState& pointer)
    {
        return std::string(TextFormat("%s at (%.0f, %.0f) moved (%+.0f, %+.0f)%s%s",
            pointer.down ? "DOWN" : "-",
            pointer.position.x, pointer.position.y,
            pointer.delta.x, pointer.delta.y,
            pointer.pressed ? " pressed" : "",
            pointer.released ? " released" : ""));
    };
    add("Primary pointer", pointerValue(primaryPointer), primaryPointer.down);
    add("Secondary pointer", pointerValue(secondaryPointer), secondaryPointer.down);
    add("Zoom", TextFormat("%+.2f", zoomDelta), zoomDelta != 0.0f);
    add("Pan", TextFormat("(%+.0f, %+.0f)", panDelta.x, panDelta.y),
        panDelta.x != 0.0f || panDelta.y != 0.0f);

    int nameWidth = 0;
    int valueWidth = 0;
    for (const Line& line : lines)
    {
        nameWidth = std::max(nameWidth, MeasureText(line.name.c_str(), fontSize));
        valueWidth = std::max(valueWidth, MeasureText(line.value.c_str(), fontSize));
    }

    const int x = 10;
    const int y = 56; // Below the FPS counter
    const int valueX = x + nameWidth + fontSize;
    DrawRectangle(x - 6, y - 6, valueX - x + valueWidth + 12,
        static_cast<int>(lines.size()) * lineHeight + 8, Color{ 0, 0, 0, 170 });
    for (int i = 0; i < static_cast<int>(lines.size()); ++i)
    {
        const Color color = lines[i].highlighted ? active : idle;
        DrawText(lines[i].name.c_str(), x, y + i * lineHeight, fontSize, color);
        DrawText(lines[i].value.c_str(), valueX, y + i * lineHeight, fontSize, color);
    }

    // Each finger: its id, and a ring colored by what owns it.
    for (int i = 0; i < backend->TouchCount(); ++i)
    {
        const int touchId = backend->TouchId(i);
        const Vector2 position = backend->TouchPosition(i);
        const bool onControl = IsTouchCaptured(touchId);
        const bool isPointer = touchId == primaryTouchId;
        const Color color = onControl ? Color{ 255, 190, 60, 230 }
            : isPointer ? Color{ 120, 230, 120, 230 }
            : Color{ 230, 120, 230, 230 };
        DrawCircleLinesV(position, 28.0f, color);
        DrawCircleLinesV(position, 30.0f, color);
        DrawText(TextFormat("%d %s", touchId,
                onControl ? "control" : isPointer ? "pointer" : "gesture"),
            static_cast<int>(position.x) + 34, static_cast<int>(position.y) - fontSize / 2,
            fontSize, color);
    }
}
