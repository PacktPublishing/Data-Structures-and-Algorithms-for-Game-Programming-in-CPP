#pragma once

#include "raylib.h"

// Everything InputSystem reads from the platform: keys, mouse, touches, screen
// size, density, time and focus. RaylibInputBackend, the default, forwards to
// raylib; tests substitute a scripted backend so input logic can run without a
// window.
class InputBackend
{
public:
    virtual ~InputBackend() = default;

    virtual bool IsKeyDown(int key) const = 0;
    virtual bool IsKeyPressed(int key) const = 0;
    virtual bool IsKeyReleased(int key) const = 0;

    virtual bool IsMouseButtonDown(int button) const = 0;
    virtual bool IsMouseButtonPressed(int button) const = 0;
    virtual bool IsMouseButtonReleased(int button) const = 0;
    virtual Vector2 MousePosition() const = 0;
    virtual Vector2 MouseDelta() const = 0;
    virtual float MouseWheelMove() const = 0;

    // Fingers currently on the screen.
    virtual int TouchCount() const = 0;
    virtual int TouchId(int index) const = 0;
    virtual Vector2 TouchPosition(int index) const = 0;

    virtual int ScreenWidth() const = 0;
    virtual int ScreenHeight() const = 0;
    // Pixels per density-independent pixel (1 dp = 1 px at 160 dpi).
    virtual float DpScale() const = 0;
    virtual double Time() const = 0;
    virtual bool IsWindowFocused() const = 0;
};

class RaylibInputBackend : public InputBackend
{
public:
    bool IsKeyDown(int key) const override;
    bool IsKeyPressed(int key) const override;
    bool IsKeyReleased(int key) const override;

    bool IsMouseButtonDown(int button) const override;
    bool IsMouseButtonPressed(int button) const override;
    bool IsMouseButtonReleased(int button) const override;
    Vector2 MousePosition() const override;
    Vector2 MouseDelta() const override;
    float MouseWheelMove() const override;

    // Real touch points on Android and on touch-screen browsers. When there
    // are none (always, on desktop) the left mouse button stands in for one
    // finger, which is what makes touch emulation usable with a mouse.
    int TouchCount() const override;
    int TouchId(int index) const override;
    Vector2 TouchPosition(int index) const override;

    int ScreenWidth() const override;
    int ScreenHeight() const override;
    // Android reports its density bucket; desktop reports the monitor's
    // content scale, which is 1 on most displays.
    float DpScale() const override;
    double Time() const override;
    bool IsWindowFocused() const override;
};
