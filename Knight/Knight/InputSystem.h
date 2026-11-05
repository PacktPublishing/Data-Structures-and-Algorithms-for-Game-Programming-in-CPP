#pragma once

#include "raylib.h"
#include "InputBackend.h"

#include <string>
#include <unordered_map>
#include <vector>

struct InputAction
{
    int id = -1;
    bool IsValid() const { return id >= 0; }
};

struct InputAxis2D
{
    int id = -1;
    bool IsValid() const { return id >= 0; }
};

struct PointerState
{
    Vector2 position = { 0, 0 };
    Vector2 delta = { 0, 0 };
    bool pressed = false;
    bool down = false;
    bool released = false;
};

enum class TouchButtonAnchor
{
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

// On-screen D-pad that can drive a 2D axis on the Touch profile.
enum class TouchPad
{
    None,
    Left,
    Right
};

// Which physical input the platform provides. Chosen once at startup:
// Android uses Touch; desktop and browser builds use KeyboardMouse and never
// read touch input or show the on-screen controls.
enum class InputProfile
{
    KeyboardMouse,
    Touch
};

// Central input service. Gameplay code reads named actions and axes; keys and
// on-screen touch controls both feed them, so no platform conditionals are
// needed. Default axes: MoveAxis() (WASD or arrows, left pad) and
// SecondaryAxis() (IJKM, right pad).
class InputSystem
{
public:
    InputSystem();

    // Replace where input comes from, e.g. a scripted backend in tests. The
    // backend must outlive the InputSystem; nullptr restores raylib.
    void SetBackend(const InputBackend* newBackend);

    InputProfile Profile() const { return profile; }
    // Override the platform default, e.g. to try the touch layout on desktop,
    // where the left mouse button acts as a single finger. Setting the
    // environment variable KNIGHT_INPUT_PROFILE=touch does the same at startup.
    void SetProfile(InputProfile newProfile);

    void BeginFrame();
    void DrawOverlay() const;
    // Diagnostic panel: every action and axis with its current state, both
    // pointers, zoom and pan, plus a marker on each finger on the screen.
    void DrawDebugOverlay() const;
    void Reset();

    // Returns the existing action if the name is already registered.
    InputAction RegisterAction(const char* name);
    // Returns an invalid action if the name is not registered.
    InputAction FindAction(const char* name) const;
    void BindKey(InputAction action, int key);
    void AddTouchButton(InputAction action, const char* label,
        TouchButtonAnchor anchor = TouchButtonAnchor::BottomRight,
        int slot = 0);

    // Returns the existing axis if the name is already registered.
    InputAxis2D RegisterAxis2D(const char* name, int upKey, int downKey,
        int leftKey, int rightKey);
    // Adds another set of keys that drive the same axis (e.g. arrows).
    void BindAxisKeys(InputAxis2D axis, int upKey, int downKey,
        int leftKey, int rightKey);
    // Pass TouchPad::None to detach. A pad no axis is bound to is hidden and
    // ignores touches; its labels come from the first axis bound to it.
    void BindAxisTouchPad(InputAxis2D axis, TouchPad pad);
    // A floating pad appears wherever the thumb lands in the lower part of its
    // half of the screen and follows the thumb; a fixed pad (the default)
    // stays in its corner.
    void SetTouchPadFloating(TouchPad pad, bool floating);

    bool Down(InputAction action) const;
    bool Pressed(InputAction action) const;
    bool Released(InputAction action) const;
    // x is right (+) / left (-), y is down (+) / up (-), screen convention.
    // Keys give -1, 0 or 1 per component; a touch pad gives an analog value
    // inside the unit circle that grows with the finger's distance from the
    // pad's center. Keys win over the pad on any component they set.
    Vector2 Axis(InputAxis2D axis) const;
    // Desktop: left mouse button. Touch: the first finger that is not on an
    // on-screen control. It is released while two such fingers are down
    // (a pinch) and does not restart until every finger has lifted.
    const PointerState& PrimaryPointer() const { return primaryPointer; }
    // Desktop: right mouse button. Touch: down and delta follow the primary
    // finger, so cameras get drag-to-orbit without platform checks, while
    // pressed fires only on a long press (the touch stand-in for right-click)
    // and released when that finger lifts.
    const PointerState& SecondaryPointer() const { return secondaryPointer; }
    // Mouse wheel movement this frame. Always 0 on the Touch profile; use
    // ZoomDelta() for zooming that should also work with a pinch.
    float WheelMove() const;
    // Desktop: mouse wheel. Touch: two-finger pinch, where spreading the
    // fingers is positive like scrolling the wheel up. Units roughly match
    // wheel notches.
    float ZoomDelta() const { return zoomDelta; }
    // Touch: movement of a two-finger drag's midpoint this frame, in pixels.
    // Always zero on desktop.
    Vector2 PanDelta() const { return panDelta; }

    // Built-in action for the Android Back button (KEY_BACK).
    InputAction BackAction() const { return backAction; }

    // Compatibility queries for code written against raylib key codes. A key
    // also reads as held while a touch control drives an action or axis bound
    // to it, so key-based samples work on touch without changes.
    bool IsKeyDown(int key) const;
    bool IsKeyPressed(int key) const;
    bool IsKeyReleased(int key) const;

    // Has no effect on the KeyboardMouse profile.
    void SetVirtualControlsVisible(bool visible);
    bool VirtualControlsVisible() const { return virtualControlsVisible; }
    int TouchButtonCount() const { return static_cast<int>(buttons.size()); }

    InputAxis2D MoveAxis() const { return moveAxis; }
    InputAxis2D SecondaryAxis() const { return secondaryAxis; }

private:
    struct ActionDefinition
    {
        std::string name;
        std::vector<int> keys;
    };

    struct AxisKeys
    {
        int upKey;
        int downKey;
        int leftKey;
        int rightKey;
    };

    struct AxisDefinition
    {
        std::string name;
        std::vector<AxisKeys> keySets;
        TouchPad pad = TouchPad::None;
    };

    struct TouchButton
    {
        InputAction action;
        std::string label;
        TouchButtonAnchor anchor;
        int slot;
        int touchId = -1;
        int previousTouchId = -1;
        Rectangle bounds = { 0, 0, 0, 0 };
    };

    struct PadState
    {
        bool floating = false;
        int touchId = -1;
        Vector2 center = { 0, 0 };   // Where the stick is drawn and measured from
        Vector2 value = { 0, 0 };    // Analog value this frame
    };

    enum class CapturedControlType
    {
        Pad,
        Button
    };

    struct TouchCapture
    {
        int touchId;
        CapturedControlType type;
        int index;      // TouchPad value for pads, button index for buttons
    };

    struct FreeTouch
    {
        int id;
        Vector2 position;
    };

    void RegisterDefaults();
    float ShortSide() const;
    void UpdateLayout();
    void UpdateTouches();
    void UpdateActions();
    void UpdateVirtualKeys();
    void UpdateMousePointers();
    void UpdateTouchPointers();
    void UpdatePinch(const std::vector<FreeTouch>& freeTouches);
    void UpdateLongPress();
    void ReleasePrimaryTouch();
    void UpdatePad(Vector2 position, TouchPad pad);
    void ResetTouchState();
    Rectangle ButtonBounds(const TouchButton& button) const;
    // Fixed position of the pad in its bottom corner.
    Rectangle PadBounds(TouchPad pad) const;
    // Area where a new touch grabs the pad.
    Rectangle PadCaptureZone(TouchPad pad) const;
    // First registered axis driven by the pad, or nullptr if none is.
    const AxisDefinition* AxisForPad(TouchPad pad) const;
    Vector2 PadValue(TouchPad pad) const;
    bool IsTouchPresent(int touchId, Vector2* position = nullptr) const;
    bool IsTouchCaptured(int touchId) const;
    int FindTouchIndexById(int touchId) const;
    bool IsValidAction(InputAction action) const;

    std::vector<ActionDefinition> actions;
    std::vector<AxisDefinition> axes;
    std::vector<TouchButton> buttons;
    std::vector<TouchCapture> captures;

    // Per-action held state, sampled once per frame for edge detection.
    std::vector<bool> actionDown;
    std::vector<bool> previousActionDown;

    // Indexed by TouchPad; entry 0 (None) stays idle.
    PadState pads[3];

    // Keys implied by touch controls, derived from action and axis state.
    std::unordered_map<int, bool> virtualKeys;
    std::unordered_map<int, bool> previousVirtualKeys;

    InputAxis2D moveAxis;
    InputAxis2D secondaryAxis;
    InputAction backAction;

    // Control layout in pixels, recomputed every frame from screen size and
    // density.
    float padSize = 0.0f;
    float buttonSize = 0.0f;
    float buttonPadding = 0.0f;
    float controlMargin = 0.0f;

    const InputBackend* backend;
    InputProfile profile;
    bool virtualControlsVisible;
    bool wasFocused = true;
    PointerState primaryPointer;
    PointerState secondaryPointer;
    int primaryTouchId = -1;
    Vector2 lastPointerPosition = { 0, 0 };

    // Gestures
    float zoomDelta = 0.0f;
    Vector2 panDelta = { 0, 0 };
    int pinchTouchIds[2] = { -1, -1 };
    float lastPinchDistance = 0.0f;
    Vector2 lastPinchMidpoint = { 0, 0 };
    bool waitForAllFingersUp = false;
    double primaryPressTime = 0.0;
    Vector2 primaryPressPosition = { 0, 0 };
    bool longPressCancelled = false;
    bool longPressFired = false;
};

