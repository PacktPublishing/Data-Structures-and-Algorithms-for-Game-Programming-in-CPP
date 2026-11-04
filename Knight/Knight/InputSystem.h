#pragma once

#include "raylib.h"

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

// Which physical input the platform provides. Chosen once at startup:
// Android uses Touch; desktop and browser builds use KeyboardMouse and never
// read touch input or show the on-screen controls.
enum class InputProfile
{
    KeyboardMouse,
    Touch
};

// Central input service. Keyboard and virtual controls are merged into the
// same action state, so gameplay code does not need platform conditionals.
class InputSystem
{
public:
    InputSystem();

    InputProfile Profile() const { return profile; }
    // Override the platform default, e.g. to try the touch layout on desktop
    // (raylib reports the left mouse button as touch point 0 there).
    void SetProfile(InputProfile newProfile);

    void BeginFrame();
    void DrawOverlay() const;
    void Reset();

    InputAction RegisterAction(const char* name);
    void BindKey(InputAction action, int key);
    void AddTouchButton(InputAction action, const char* label,
        TouchButtonAnchor anchor = TouchButtonAnchor::BottomRight,
        int slot = 0);

    InputAxis2D RegisterAxis2D(const char* name, int upKey, int downKey,
        int leftKey, int rightKey);

    bool Down(InputAction action) const;
    bool Pressed(InputAction action) const;
    bool Released(InputAction action) const;
    Vector2 Axis(InputAxis2D axis) const;
    const PointerState& PrimaryPointer() const { return primaryPointer; }
    // Desktop: right mouse button. Touch platforms: the same uncaptured touch
    // used by PrimaryPointer. Cameras can therefore support drag-to-orbit
    // without platform checks in gameplay code.
    const PointerState& SecondaryPointer() const { return secondaryPointer; }
    // Mouse wheel movement this frame. Always 0 on the Touch profile.
    float WheelMove() const;

    // Compatibility queries make existing raylib call sites easy to migrate.
    // They also include keys emitted by virtual D-pads and buttons.
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

    struct AxisDefinition
    {
        std::string name;
        int upKey;
        int downKey;
        int leftKey;
        int rightKey;
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

    enum class CapturedControlType
    {
        LeftDPad,
        RightDPad,
        Button
    };

    struct TouchCapture
    {
        int touchId;
        CapturedControlType type;
        int index;
    };

    void UpdateLayout();
    void UpdateTouches();
    void UpdatePrimaryPointer();
    void UpdateSecondaryPointer();
    void UpdateDPad(Vector2 position, bool leftPad);
    void PressVirtualKey(int key);
    void ResetPointers();
    Rectangle ButtonBounds(const TouchButton& button) const;
    bool IsTouchPresent(int touchId, Vector2* position = nullptr) const;
    bool IsTouchCaptured(int touchId) const;
    int FindTouchIndexById(int touchId) const;
    bool ActionVirtualDown(InputAction action) const;

    std::vector<ActionDefinition> actions;
    std::vector<AxisDefinition> axes;
    std::vector<TouchButton> buttons;
    std::vector<TouchCapture> captures;

    std::unordered_map<int, bool> virtualKeys;
    std::unordered_map<int, bool> previousVirtualKeys;

    InputAxis2D moveAxis;
    InputAxis2D secondaryAxis;

    Rectangle leftDPadBounds = { 0, 0, 0, 0 };
    Rectangle rightDPadBounds = { 0, 0, 0, 0 };
    InputProfile profile;
    bool virtualControlsVisible;
    PointerState primaryPointer;
    PointerState secondaryPointer;
    int primaryTouchId = -1;
    Vector2 lastPointerPosition = { 0, 0 };
};

