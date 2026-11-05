#include "InputBackend.h"

bool RaylibInputBackend::IsKeyDown(int key) const { return ::IsKeyDown(key); }
bool RaylibInputBackend::IsKeyPressed(int key) const { return ::IsKeyPressed(key); }
bool RaylibInputBackend::IsKeyReleased(int key) const { return ::IsKeyReleased(key); }

bool RaylibInputBackend::IsMouseButtonDown(int button) const { return ::IsMouseButtonDown(button); }
bool RaylibInputBackend::IsMouseButtonPressed(int button) const { return ::IsMouseButtonPressed(button); }
bool RaylibInputBackend::IsMouseButtonReleased(int button) const { return ::IsMouseButtonReleased(button); }
Vector2 RaylibInputBackend::MousePosition() const { return ::GetMousePosition(); }
Vector2 RaylibInputBackend::MouseDelta() const { return ::GetMouseDelta(); }
float RaylibInputBackend::MouseWheelMove() const { return ::GetMouseWheelMove(); }

#if defined(PLATFORM_ANDROID)
int RaylibInputBackend::TouchCount() const { return ::GetTouchPointCount(); }
int RaylibInputBackend::TouchId(int index) const { return ::GetTouchPointId(index); }
Vector2 RaylibInputBackend::TouchPosition(int index) const { return ::GetTouchPosition(index); }
#else
int RaylibInputBackend::TouchCount() const { return ::IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 1 : 0; }
int RaylibInputBackend::TouchId(int) const { return 0; }
Vector2 RaylibInputBackend::TouchPosition(int) const { return ::GetMousePosition(); }
#endif

int RaylibInputBackend::ScreenWidth() const { return ::GetScreenWidth(); }
int RaylibInputBackend::ScreenHeight() const { return ::GetScreenHeight(); }

float RaylibInputBackend::DpScale() const
{
    const float scale = ::GetWindowScaleDPI().x;
    return scale > 0.0f ? scale : 1.0f;
}

double RaylibInputBackend::Time() const { return ::GetTime(); }
bool RaylibInputBackend::IsWindowFocused() const { return ::IsWindowFocused(); }
