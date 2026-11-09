// Unit tests for InputSystem. They drive it through a scripted InputBackend, so
// they run without a window, a GPU or a touch screen. Build the
// knight_input_tests target and run it directly or through ctest.

#include "InputSystem.h"
#include "Display.h"

#include <cmath>
#include <cstdio>
#include <set>
#include <vector>

namespace
{
    int failures = 0;
    int checks = 0;

#define CHECK(condition)                                                    \
    do {                                                                    \
        ++checks;                                                           \
        if (!(condition)) {                                                 \
            ++failures;                                                     \
            std::printf("  FAILED %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        }                                                                   \
    } while (0)

#define CHECK_NEAR(actual, expected, tolerance) \
    CHECK(std::fabs((actual) - (expected)) <= (tolerance))

    // Scripted platform. Tests change keys, buttons and fingers, then call
    // Frame(), which plays the role of raylib polling input between frames:
    // "pressed" means down this frame and not the previous one.
    class FakeBackend : public InputBackend
    {
    public:
        std::set<int> keys;
        std::set<int> buttons;
        struct Finger { int id; Vector2 position; };
        std::vector<Finger> fingers;
        Vector2 mouse = { 0, 0 };
        float wheel = 0.0f;
        int width = 1280;
        int height = 720;
        float dpScale = 1.0f;
        double time = 0.0;
        bool focused = true;
        // Empty: the whole screen.
        Rectangle controlArea = { 0, 0, 0, 0 };

        void Frame(InputSystem& input, double seconds = 1.0 / 60.0)
        {
            previousKeys = shownKeys;
            shownKeys = keys;
            previousButtons = shownButtons;
            shownButtons = buttons;
            shownFingers = fingers;
            mouseDelta = Vector2{ mouse.x - shownMouse.x, mouse.y - shownMouse.y };
            shownMouse = mouse;
            shownWheel = wheel;
            wheel = 0.0f;
            time += seconds;
            input.BeginFrame();
        }

        void Touch(int id, float x, float y)
        {
            for (Finger& finger : fingers)
            {
                if (finger.id == id)
                {
                    finger.position = Vector2{ x, y };
                    return;
                }
            }
            fingers.push_back(Finger{ id, Vector2{ x, y } });
        }

        void Lift(int id)
        {
            for (size_t i = 0; i < fingers.size(); ++i)
            {
                if (fingers[i].id == id)
                {
                    fingers.erase(fingers.begin() + i);
                    return;
                }
            }
        }

        bool IsKeyDown(int key) const override { return shownKeys.count(key) > 0; }
        bool IsKeyPressed(int key) const override { return shownKeys.count(key) > 0 && previousKeys.count(key) == 0; }
        bool IsKeyReleased(int key) const override { return shownKeys.count(key) == 0 && previousKeys.count(key) > 0; }

        bool IsMouseButtonDown(int button) const override { return shownButtons.count(button) > 0; }
        bool IsMouseButtonPressed(int button) const override { return shownButtons.count(button) > 0 && previousButtons.count(button) == 0; }
        bool IsMouseButtonReleased(int button) const override { return shownButtons.count(button) == 0 && previousButtons.count(button) > 0; }
        Vector2 MousePosition() const override { return shownMouse; }
        Vector2 MouseDelta() const override { return mouseDelta; }
        float MouseWheelMove() const override { return shownWheel; }

        int TouchCount() const override { return static_cast<int>(shownFingers.size()); }
        int TouchId(int index) const override { return shownFingers[index].id; }
        Vector2 TouchPosition(int index) const override { return shownFingers[index].position; }

        int ScreenWidth() const override { return width; }
        int ScreenHeight() const override { return height; }
        float DpScale() const override { return dpScale; }
        Rectangle ControlArea() const override
        {
            return controlArea.width > 0 ? controlArea : InputBackend::ControlArea();
        }
        double Time() const override { return time; }
        bool IsWindowFocused() const override { return focused; }

    private:
        std::set<int> shownKeys, previousKeys;
        std::set<int> shownButtons, previousButtons;
        std::vector<Finger> shownFingers;
        Vector2 shownMouse = { 0, 0 };
        Vector2 mouseDelta = { 0, 0 };
        float shownWheel = 0.0f;
    };

    // Layout at 1280x720, 1 px per dp and control scale 1 (see the constants
    // in InputSystem.cpp): pads are 130 px with a 16 px margin, buttons are
    // 56 px.
    const Vector2 LeftPadCenter = { 16 + 65, 720 - 16 - 65 };
    const Vector2 RightPadCenter = { 1280 - 16 - 65, 720 - 16 - 65 };
    const float PadRadius = 65.0f;
    const Vector2 TopRightButton0 = { 1280 - 16 - 28, 16 + 28 };
    const Vector2 OpenArea = { 640, 300 };

    struct Fixture
    {
        FakeBackend platform;
        InputSystem input;

        // withPads gives the two-pad layout most tests exercise; InputSystem
        // itself starts with no on-screen controls.
        explicit Fixture(InputProfile profile = InputProfile::Touch, bool withPads = true)
        {
            input.SetBackend(&platform);
            input.SetProfile(profile);
            input.SetControlScale(1.0f);
            if (withPads)
            {
                input.BindAxisTouchPad(input.MoveAxis(), TouchPad::Left);
                input.BindAxisTouchPad(input.SecondaryAxis(), TouchPad::Right);
            }
        }
    };

    void KeyboardActionEdges()
    {
        Fixture f(InputProfile::KeyboardMouse);
        InputAction jump = f.input.RegisterAction("Jump");
        f.input.BindKey(jump, KEY_SPACE);

        f.platform.Frame(f.input);
        CHECK(!f.input.Down(jump));

        f.platform.keys.insert(KEY_SPACE);
        f.platform.Frame(f.input);
        CHECK(f.input.Pressed(jump));
        CHECK(f.input.Down(jump));

        f.platform.Frame(f.input);
        CHECK(!f.input.Pressed(jump));
        CHECK(f.input.Down(jump));

        f.platform.keys.erase(KEY_SPACE);
        f.platform.Frame(f.input);
        CHECK(f.input.Released(jump));
        CHECK(!f.input.Down(jump));

        // Registering the same name again returns the same action.
        CHECK(f.input.RegisterAction("Jump").id == jump.id);
        CHECK(f.input.FindAction("Jump").id == jump.id);
        CHECK(!f.input.FindAction("Missing").IsValid());
    }

    void TouchButtonDrivesActionAndKey()
    {
        Fixture f;
        InputAction attack = f.input.RegisterAction("Attack");
        f.input.BindKey(attack, KEY_SPACE);
        f.input.AddTouchButton(attack, "ATTACK", TouchButtonAnchor::TopRight, 0);

        f.platform.Frame(f.input);
        f.platform.Touch(1, TopRightButton0.x, TopRightButton0.y);
        f.platform.Frame(f.input);
        CHECK(f.input.Pressed(attack));
        CHECK(f.input.Down(attack));
        // Key-based code sees the bound key as pressed too.
        CHECK(f.input.IsKeyPressed(KEY_SPACE));
        CHECK(f.input.IsKeyDown(KEY_SPACE));
        // A finger on a control is not a pointer.
        CHECK(!f.input.PrimaryPointer().down);

        f.platform.Frame(f.input);
        CHECK(!f.input.Pressed(attack));
        CHECK(f.input.Down(attack));
        CHECK(!f.input.IsKeyPressed(KEY_SPACE));

        f.platform.Lift(1);
        f.platform.Frame(f.input);
        CHECK(f.input.Released(attack));
        CHECK(f.input.IsKeyReleased(KEY_SPACE));
        CHECK(!f.input.Down(attack));
    }

    void PadGivesAnalogAxisAndDirectionKeys()
    {
        Fixture f;
        const InputAxis2D move = f.input.MoveAxis();

        // Inside the dead zone: no movement.
        f.platform.Touch(1, LeftPadCenter.x + 5, LeftPadCenter.y);
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(move).x == 0.0f && f.input.Axis(move).y == 0.0f);

        // At the top edge: full strength up, and W plus Up read as held.
        f.platform.Touch(1, LeftPadCenter.x, LeftPadCenter.y - PadRadius);
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.Axis(move).x, 0.0f, 1e-4f);
        CHECK_NEAR(f.input.Axis(move).y, -1.0f, 1e-4f);
        CHECK(f.input.IsKeyDown(KEY_W));
        CHECK(f.input.IsKeyDown(KEY_UP));
        CHECK(!f.input.IsKeyDown(KEY_A));

        // Halfway between the dead zone and the edge: half strength.
        const float deadZone = 130.0f * 0.14f;
        f.platform.Touch(1, LeftPadCenter.x, LeftPadCenter.y - (deadZone + (PadRadius - deadZone) * 0.5f));
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.Axis(move).y, -0.5f, 1e-3f);

        // Up-right diagonal presses both direction keys.
        f.platform.Touch(1, LeftPadCenter.x + 40, LeftPadCenter.y - 40);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_W));
        CHECK(f.input.IsKeyDown(KEY_D));
        CHECK(f.input.Axis(move).x > 0.5f && f.input.Axis(move).y < -0.5f);

        // Beyond the edge stays at full strength.
        f.platform.Touch(1, LeftPadCenter.x + 400, LeftPadCenter.y);
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.Axis(move).x, 1.0f, 1e-4f);

        f.platform.Lift(1);
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(move).x == 0.0f && f.input.Axis(move).y == 0.0f);
        CHECK(f.input.IsKeyReleased(KEY_D));
    }

    void KeysWinOverPadPerComponent()
    {
        Fixture f;
        const InputAxis2D move = f.input.MoveAxis();
        f.platform.Touch(1, LeftPadCenter.x + PadRadius, LeftPadCenter.y); // pad: right
        f.platform.keys.insert(KEY_A);                                       // key: left
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(move).x == -1.0f);
    }

    void UnboundPadIgnoresTouches()
    {
        Fixture f;
        f.input.BindAxisTouchPad(f.input.MoveAxis(), TouchPad::None);
        f.platform.Touch(1, LeftPadCenter.x, LeftPadCenter.y - PadRadius);
        f.platform.Frame(f.input);
        CHECK(!f.input.IsKeyDown(KEY_W));
        CHECK(f.input.PrimaryPointer().pressed); // the finger is a plain pointer
    }

    void PointerSkipsControlsAndKeepsItsFinger()
    {
        Fixture f;

        // Thumb on the pad, then a second finger in the open: the second one
        // is the pointer.
        f.platform.Touch(1, LeftPadCenter.x, LeftPadCenter.y);
        f.platform.Frame(f.input);
        CHECK(!f.input.PrimaryPointer().down);
        f.platform.Touch(1, LeftPadCenter.x + 40, LeftPadCenter.y);
        f.platform.Frame(f.input);
        CHECK(!f.input.SecondaryPointer().down);
        CHECK(f.input.SecondaryPointer().delta.x == 0.0f);
        CHECK(f.input.SecondaryPointer().delta.y == 0.0f);
        f.platform.Touch(2, OpenArea.x, OpenArea.y);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().pressed);
        CHECK(f.input.PrimaryPointer().position.x == OpenArea.x);

        // Only the free finger supplies camera delta while the pad remains held.
        f.platform.Touch(2, OpenArea.x + 30, OpenArea.y + 10);
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.SecondaryPointer().delta.x, 30.0f, 1e-4f);
        CHECK_NEAR(f.input.SecondaryPointer().delta.y, 10.0f, 1e-4f);

        // Dragging the pointer onto the right pad does not hand it to the pad.
        f.platform.Touch(2, RightPadCenter.x, RightPadCenter.y - PadRadius);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().down);
        CHECK(f.input.PrimaryPointer().delta.x > 0.0f);
        CHECK(!f.input.IsKeyDown(KEY_I));

        f.platform.Lift(2);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().released);
    }

    void PinchZoomsAndSuspendsPointer()
    {
        Fixture f;
        f.platform.Touch(1, 600, 300);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().pressed);

        // The second finger ends the drag; the first pinch frame only records.
        f.platform.Touch(2, 700, 300);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().released);
        CHECK(f.input.ZoomDelta() == 0.0f);

        // Spreading by 72 px (a tenth of the 720 px short side) is one notch.
        f.platform.Touch(1, 564, 310);
        f.platform.Touch(2, 736, 310);
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.ZoomDelta(), 1.0f, 1e-3f);
        CHECK_NEAR(f.input.PanDelta().y, 10.0f, 1e-3f);
        CHECK(!f.input.PrimaryPointer().down);

        // Pinching in is negative.
        f.platform.Touch(1, 600, 310);
        f.platform.Touch(2, 700, 310);
        f.platform.Frame(f.input);
        CHECK(f.input.ZoomDelta() < 0.0f);

        // The finger left after a pinch does not start a drag...
        f.platform.Lift(2);
        f.platform.Frame(f.input);
        CHECK(!f.input.PrimaryPointer().pressed && !f.input.PrimaryPointer().down);
        CHECK(f.input.ZoomDelta() == 0.0f);

        // ...until every finger has lifted.
        f.platform.Lift(1);
        f.platform.Frame(f.input);
        f.platform.Touch(3, 600, 300);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().pressed);
    }

    void MouseWheelIsZoomOnDesktop()
    {
        Fixture f(InputProfile::KeyboardMouse);
        f.platform.wheel = 2.0f;
        f.platform.Frame(f.input);
        CHECK(f.input.ZoomDelta() == 2.0f);
        CHECK(f.input.WheelMove() == 2.0f);
        CHECK(f.input.PanDelta().x == 0.0f);
    }

    void LongPressIsRightClick()
    {
        Fixture f;

        // A quick tap is not a right-click, though drag-to-orbit still sees it.
        f.platform.Touch(1, OpenArea.x, OpenArea.y);
        f.platform.Frame(f.input);
        CHECK(f.input.SecondaryPointer().down);
        CHECK(!f.input.SecondaryPointer().pressed);
        f.platform.Lift(1);
        f.platform.Frame(f.input);
        CHECK(!f.input.SecondaryPointer().released);

        // Holding still for half a second fires once, and lifting releases.
        f.platform.Touch(2, OpenArea.x, OpenArea.y);
        f.platform.Frame(f.input);
        f.platform.Frame(f.input, 0.3);
        CHECK(!f.input.SecondaryPointer().pressed);
        f.platform.Frame(f.input, 0.3);
        CHECK(f.input.SecondaryPointer().pressed);
        f.platform.Frame(f.input, 0.3);
        CHECK(!f.input.SecondaryPointer().pressed);
        f.platform.Lift(2);
        f.platform.Frame(f.input);
        CHECK(f.input.SecondaryPointer().released);

        // Moving past the slop turns it into a drag for good.
        f.platform.Touch(3, OpenArea.x, OpenArea.y);
        f.platform.Frame(f.input);
        f.platform.Touch(3, OpenArea.x + 30, OpenArea.y);
        f.platform.Frame(f.input, 0.1);
        f.platform.Touch(3, OpenArea.x, OpenArea.y);
        f.platform.Frame(f.input, 1.0);
        CHECK(!f.input.SecondaryPointer().pressed);
    }

    void FocusLossReleasesEverything()
    {
        Fixture f;
        InputAction attack = f.input.RegisterAction("Attack");
        f.input.AddTouchButton(attack, "ATTACK", TouchButtonAnchor::TopRight, 0);

        f.platform.Touch(1, TopRightButton0.x, TopRightButton0.y);
        f.platform.Touch(2, LeftPadCenter.x, LeftPadCenter.y - PadRadius);
        f.platform.Frame(f.input);
        CHECK(f.input.Down(attack));
        CHECK(f.input.IsKeyDown(KEY_W));

        // The app goes to the background; its fingers never report lifting.
        f.platform.focused = false;
        f.platform.fingers.clear();
        f.platform.Frame(f.input);
        CHECK(!f.input.Down(attack));
        CHECK(f.input.Released(attack));
        CHECK(!f.input.IsKeyDown(KEY_W));
    }

    void KeyboardProfileIgnoresTouch()
    {
        Fixture f(InputProfile::KeyboardMouse);
        InputAction attack = f.input.RegisterAction("Attack");
        f.input.AddTouchButton(attack, "ATTACK", TouchButtonAnchor::TopRight, 0);
        f.input.SetVirtualControlsVisible(true);
        CHECK(!f.input.VirtualControlsVisible());

        f.platform.Touch(1, TopRightButton0.x, TopRightButton0.y);
        f.platform.Frame(f.input);
        CHECK(!f.input.Down(attack));
    }

    void ControlsScaleWithDensity()
    {
        // At 2 px per dp the pad doubles to 260 px, so a finger 100 px above
        // its new center is inside it but outside the 1x pad.
        Fixture f;
        f.platform.dpScale = 2.0f;
        const Vector2 center = { 16 * 2 + 130, 720 - 16 * 2 - 130 };
        f.platform.Touch(1, center.x, center.y - 100);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_W));

        // On a small screen the pad is capped at half the short side.
        Fixture small;
        small.platform.width = 400;
        small.platform.height = 240;
        small.platform.dpScale = 3.0f;
        const float padSize = 240 * 0.5f;
        const float scale = padSize / 130.0f;
        const Vector2 smallCenter = { 16 * scale + padSize / 2, 240 - 16 * scale - padSize / 2 };
        // Just inside the top edge: (0.49 - 0.14) / (0.5 - 0.14) of full strength.
        small.platform.Touch(1, smallCenter.x, smallCenter.y - padSize * 0.49f);
        small.platform.Frame(small.input);
        CHECK_NEAR(small.input.Axis(small.input.MoveAxis()).y, -0.35f / 0.36f, 1e-3f);
    }

    void ControlScaleResizesControls()
    {
        // A fresh InputSystem draws controls at 1.2x: a 156 px pad and
        // 67.2 px buttons with a 19.2 px margin at 1 px per dp.
        InputSystem fresh;
        CHECK(fresh.ControlScale() == InputSystem::DefaultControlScale);
        CHECK(InputSystem::DefaultControlScale == 1.2f);

        Fixture f;
        f.input.SetControlScale(1.2f);
        const InputAction attack = f.input.AddKeyButton(KEY_SPACE, "ATTACK",
            TouchButtonAnchor::TopRight);
        // Inside the 1.2x pad but outside the 1x one.
        const Vector2 bigPadCenter = { 19.2f + 78, 720 - 19.2f - 78 };
        f.platform.Touch(1, bigPadCenter.x, bigPadCenter.y - 70);
        // Inside the 1.2x button but outside the 1x one.
        f.platform.Touch(2, 1200, 80);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_W));
        CHECK(f.input.Down(attack));

        // Reset keeps the scale; tiny values are raised to the minimum.
        f.input.Reset();
        CHECK(f.input.ControlScale() == 1.2f);
        f.input.SetControlScale(0.0f);
        CHECK(f.input.ControlScale() == InputSystem::MinControlScale);
    }

    void ControlsStayInsideControlArea()
    {
        // A notch on the left and system bars top and bottom: 100 px left
        // and right, 40 px top and bottom.
        Fixture f;
        f.platform.controlArea = Rectangle{ 100, 40, 1080, 640 };
        const InputAction attack = f.input.AddKeyButton(KEY_SPACE, "ATTACK",
            TouchButtonAnchor::TopRight);

        // The pad's center moves in by the insets: (100 + 16 + 65, 680 - 16 - 65).
        const Vector2 padCenter = { 181, 599 };
        f.platform.Touch(1, padCenter.x, padCenter.y - PadRadius);
        // Top-right button, slot 0: (1180 - 16 - 28, 40 + 16 + 28).
        f.platform.Touch(2, 1136, 84);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_W));
        CHECK(f.input.Down(attack));
        f.platform.Lift(1);
        f.platform.Lift(2);
        f.platform.Frame(f.input);

        // The old full-screen spots are now under the notch: nothing grabs them.
        f.platform.Touch(1, LeftPadCenter.x - 40, LeftPadCenter.y);
        f.platform.Touch(2, TopRightButton0.x, TopRightButton0.y);
        f.platform.Frame(f.input);
        CHECK(!f.input.IsKeyDown(KEY_A));
        CHECK(!f.input.Down(attack));
    }

    void FloatingPadStaysInsideControlArea()
    {
        // Tabletop posture: controls only on the lower half.
        Fixture f;
        f.platform.controlArea = Rectangle{ 60, 360, 1160, 360 };
        f.input.SetTouchPadFloating(TouchPad::Left, true);

        // A thumb on the bezel side of the area, left of x = 60, still grabs
        // the pad, which appears at the area's edge.
        f.platform.Touch(1, 20, 600);
        f.platform.Frame(f.input);
        // Sliding right by the pad radius from the clamped center is full right.
        f.platform.Touch(1, 60 + PadRadius, 600);
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.Axis(f.input.MoveAxis()).x, 1.0f, 1e-3f);

        // Above the hinge is outside the floating zone.
        f.platform.Lift(1);
        f.platform.Frame(f.input);
        f.platform.Touch(1, 300, 200);
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(f.input.MoveAxis()).x == 0.0f);
        CHECK(f.input.PrimaryPointer().down);
    }

    void DisplayComposesSafeAndControlAreas()
    {
        PlatformDisplayReport report;
        report.insetLeft = 100;
        report.insetTop = 40;
        report.insetRight = 80;
        report.insetBottom = 30;
        DisplayInfo info = Display::Compose(2400, 1080, 2.0f, report);
        CHECK(info.width == 2400 && info.height == 1080);
        CHECK(info.dpScale == 2.0f);
        CHECK(info.safeArea.x == 100 && info.safeArea.y == 40);
        CHECK(info.safeArea.width == 2400 - 180 && info.safeArea.height == 1080 - 70);
        // Not folded: controls use the whole safe area.
        CHECK(info.ControlArea().y == info.safeArea.y);
        CHECK(info.ControlArea().height == info.safeArea.height);

        // Java's density wins over raylib's.
        report.dpScale = 2.625f;
        CHECK(Display::Compose(2400, 1080, 2.0f, report).dpScale == 2.625f);

        // Tabletop: a separating horizontal hinge leaves the lower part.
        report.fold = FoldState::HalfOpened;
        report.hingeAxis = HingeAxis::Horizontal;
        report.hinge = Rectangle{ 0, 530, 2400, 20 };
        report.hingeSeparates = true;
        info = Display::Compose(2400, 1080, 2.0f, report);
        const Rectangle control = info.ControlArea();
        CHECK(control.y == 550);
        CHECK(control.height == 1050 - 550);
        CHECK(control.x == 100 && control.width == info.safeArea.width);

        // A vertical (book-style) hinge leaves the control area alone.
        report.hingeAxis = HingeAxis::Vertical;
        report.hinge = Rectangle{ 1200, 0, 0, 1080 };
        info = Display::Compose(2400, 1080, 2.0f, report);
        CHECK(info.ControlArea().y == info.safeArea.y);

        // Insets that leave nothing are ignored.
        PlatformDisplayReport broken;
        broken.insetLeft = 1300;
        broken.insetRight = 1300;
        info = Display::Compose(2400, 1080, 0.0f, broken);
        CHECK(info.safeArea.x == 0 && info.safeArea.width == 2400);
        CHECK(info.dpScale == 1.0f);

        // Equality drives OnDisplayChange: any field counts.
        DisplayInfo a = Display::Compose(1280, 720, 1.0f, PlatformDisplayReport{});
        DisplayInfo b = a;
        CHECK(a == b);
        b.fold = FoldState::Flat;
        CHECK(a != b);
    }

    void FloatingPadCentersOnThumb()
    {
        Fixture f;
        f.input.SetTouchPadFloating(TouchPad::Left, true);

        // Landing anywhere in the lower-left zone centers the pad there.
        f.platform.Touch(1, 300, 500);
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(f.input.MoveAxis()).x == 0.0f);
        CHECK(!f.input.PrimaryPointer().down);

        f.platform.Touch(1, 300 + PadRadius, 500);
        f.platform.Frame(f.input);
        CHECK_NEAR(f.input.Axis(f.input.MoveAxis()).x, 1.0f, 1e-4f);

        // Sliding far right drags the base along, so going back left
        // responds at once.
        f.platform.Touch(1, 600, 500);
        f.platform.Frame(f.input);
        f.platform.Touch(1, 600 - PadRadius - 20, 500);
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(f.input.MoveAxis()).x < 0.0f);

        // The upper part of the screen is not part of the zone.
        f.platform.Lift(1);
        f.platform.Frame(f.input);
        f.platform.Touch(2, 300, 100);
        f.platform.Frame(f.input);
        CHECK(f.input.PrimaryPointer().pressed);
    }

    void BackActionIsBoundToBackKey()
    {
        Fixture f;
        f.platform.keys.insert(KEY_BACK);
        f.platform.Frame(f.input);
        CHECK(f.input.Pressed(f.input.BackAction()));
    }

    void ResetRestoresDefaults()
    {
        Fixture f;
        f.input.RegisterAction("Attack");
        f.input.AddKeyButton(KEY_R);
        f.input.Reset();
        CHECK(!f.input.FindAction("Attack").IsValid());
        CHECK(f.input.BackAction().IsValid());
        CHECK(f.input.TouchButtonCount() == 0);
        CHECK(!f.input.IsTouchPadInUse(TouchPad::Left));

        // The default axes keep their keys.
        f.platform.keys.insert(KEY_W);
        f.platform.Frame(f.input);
        CHECK(f.input.Axis(f.input.MoveAxis()).y == -1.0f);
    }

    void NoControlsUntilSampleCreatesThem()
    {
        Fixture f(InputProfile::Touch, false);
        CHECK(!f.input.IsTouchPadInUse(TouchPad::Left));
        CHECK(!f.input.IsTouchPadInUse(TouchPad::Right));
        CHECK(f.input.TouchButtonCount() == 0);

        // A finger where the pad used to be is a plain pointer.
        f.platform.Touch(1, LeftPadCenter.x, LeftPadCenter.y - PadRadius);
        f.platform.Frame(f.input);
        CHECK(!f.input.IsKeyDown(KEY_W));
        CHECK(f.input.PrimaryPointer().pressed);
    }

    void KeyButtonActsAsKey()
    {
        Fixture f(InputProfile::Touch, false);
        const InputAction reset = f.input.AddKeyButton(KEY_R, "RESET", TouchButtonAnchor::TopLeft);
        CHECK(reset.IsValid());
        CHECK(f.input.FindAction("Key RESET").id == reset.id);

        const Vector2 topLeft0 = { 16 + 28, 16 + 28 };
        f.platform.Touch(1, topLeft0.x, topLeft0.y);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyPressed(KEY_R));
        CHECK(f.input.IsKeyDown(KEY_R));
        CHECK(!f.input.IsKeyDown(KEY_SPACE));
        CHECK(!f.input.PrimaryPointer().down);

        // The physical key drives the same action.
        f.platform.Lift(1);
        f.platform.keys.insert(KEY_R);
        f.platform.Frame(f.input);
        CHECK(f.input.Down(reset));
    }

    void AutoSlotsFillEachCornerSeparately()
    {
        Fixture f(InputProfile::Touch, false);
        f.input.AddKeyButton(KEY_ONE, "1", TouchButtonAnchor::TopRight);
        f.input.AddKeyButton(KEY_TWO, "2", TouchButtonAnchor::TopRight);
        f.input.AddKeyButton(KEY_THREE, "3", TouchButtonAnchor::BottomLeft);

        // Slot 1 in the top-right corner is one button stride (56 + 12)
        // further from the right edge than slot 0.
        f.platform.Touch(1, TopRightButton0.x - 68, TopRightButton0.y);
        f.platform.Touch(2, 16 + 28, 720 - 16 - 28);
        f.platform.Frame(f.input);
        CHECK(!f.input.IsKeyDown(KEY_ONE));
        CHECK(f.input.IsKeyDown(KEY_TWO));
        CHECK(f.input.IsKeyDown(KEY_THREE));

        f.platform.fingers.clear();
        f.platform.Touch(3, TopRightButton0.x, TopRightButton0.y);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_ONE));
    }

    void KeyPadPressesOnlyItsKeys()
    {
        Fixture f(InputProfile::Touch, false);
        const InputAxis2D camera = f.input.AddKeyPad(TouchPad::Left, "Camera",
            KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);
        CHECK(f.input.IsTouchPadInUse(TouchPad::Left));
        CHECK(!f.input.IsTouchPadInUse(TouchPad::Right));

        f.platform.Touch(1, LeftPadCenter.x, LeftPadCenter.y - PadRadius);
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_UP));
        CHECK(!f.input.IsKeyDown(KEY_W));
        CHECK_NEAR(f.input.Axis(camera).y, -1.0f, 1e-4f);

        // A pad drives one axis: binding another to it detaches the first.
        f.platform.Lift(1);
        f.platform.Frame(f.input);
        f.input.AddKeyPad(TouchPad::Left, "Depth", KEY_W, KEY_S, KEY_NULL, KEY_NULL);
        f.platform.Touch(2, LeftPadCenter.x + 40, LeftPadCenter.y - 40); // up-right
        f.platform.Frame(f.input);
        CHECK(f.input.IsKeyDown(KEY_W));
        CHECK(!f.input.IsKeyDown(KEY_UP));
        CHECK(!f.input.IsKeyDown(KEY_NULL));
    }

    void ClearVirtualControlsKeepsKeys()
    {
        Fixture f;
        const InputAction jump = f.input.AddKeyButton(KEY_SPACE, "JUMP");
        f.input.ClearVirtualControls();
        CHECK(f.input.TouchButtonCount() == 0);
        CHECK(!f.input.IsTouchPadInUse(TouchPad::Left));

        f.platform.keys.insert(KEY_SPACE);
        f.platform.Frame(f.input);
        CHECK(f.input.Down(jump));
    }

    // Touch emulation (F4) switches profiles at runtime: the sample's controls
    // appear and disappear without being recreated, and keys keep working.
    void ProfileSwitchShowsAndHidesControls()
    {
        Fixture f(InputProfile::KeyboardMouse, false);
        const InputAction jump = f.input.AddKeyButton(KEY_SPACE, "JUMP", TouchButtonAnchor::TopRight);
        CHECK(!f.input.VirtualControlsVisible());

        f.input.SetProfile(InputProfile::Touch);
        CHECK(f.input.VirtualControlsVisible());
        f.platform.Touch(1, TopRightButton0.x, TopRightButton0.y);
        f.platform.Frame(f.input);
        CHECK(f.input.Down(jump));

        // Switching back mid-press releases the button.
        f.input.SetProfile(InputProfile::KeyboardMouse);
        f.platform.Frame(f.input);
        CHECK(!f.input.VirtualControlsVisible());
        CHECK(!f.input.Down(jump));
        CHECK(f.input.TouchButtonCount() == 1);

        f.platform.Lift(1);
        f.platform.keys.insert(KEY_SPACE);
        f.platform.Frame(f.input);
        CHECK(f.input.Down(jump));
    }

    void WheelZoomsWhenTouchIsEmulated()
    {
        Fixture f;
        f.platform.wheel = -1.0f;
        f.platform.Frame(f.input);
        CHECK(f.input.ZoomDelta() == -1.0f);
        // The raw wheel stays a desktop-only reading.
        CHECK(f.input.WheelMove() == 0.0f);
    }

    struct Test { const char* name; void (*run)(); };
}

int main()
{
    const Test tests[] = {
        { "KeyboardActionEdges", KeyboardActionEdges },
        { "TouchButtonDrivesActionAndKey", TouchButtonDrivesActionAndKey },
        { "PadGivesAnalogAxisAndDirectionKeys", PadGivesAnalogAxisAndDirectionKeys },
        { "KeysWinOverPadPerComponent", KeysWinOverPadPerComponent },
        { "UnboundPadIgnoresTouches", UnboundPadIgnoresTouches },
        { "PointerSkipsControlsAndKeepsItsFinger", PointerSkipsControlsAndKeepsItsFinger },
        { "PinchZoomsAndSuspendsPointer", PinchZoomsAndSuspendsPointer },
        { "MouseWheelIsZoomOnDesktop", MouseWheelIsZoomOnDesktop },
        { "LongPressIsRightClick", LongPressIsRightClick },
        { "FocusLossReleasesEverything", FocusLossReleasesEverything },
        { "KeyboardProfileIgnoresTouch", KeyboardProfileIgnoresTouch },
        { "ControlsScaleWithDensity", ControlsScaleWithDensity },
        { "ControlScaleResizesControls", ControlScaleResizesControls },
        { "ControlsStayInsideControlArea", ControlsStayInsideControlArea },
        { "FloatingPadStaysInsideControlArea", FloatingPadStaysInsideControlArea },
        { "DisplayComposesSafeAndControlAreas", DisplayComposesSafeAndControlAreas },
        { "FloatingPadCentersOnThumb", FloatingPadCentersOnThumb },
        { "BackActionIsBoundToBackKey", BackActionIsBoundToBackKey },
        { "ResetRestoresDefaults", ResetRestoresDefaults },
        { "NoControlsUntilSampleCreatesThem", NoControlsUntilSampleCreatesThem },
        { "KeyButtonActsAsKey", KeyButtonActsAsKey },
        { "AutoSlotsFillEachCornerSeparately", AutoSlotsFillEachCornerSeparately },
        { "KeyPadPressesOnlyItsKeys", KeyPadPressesOnlyItsKeys },
        { "ClearVirtualControlsKeepsKeys", ClearVirtualControlsKeepsKeys },
        { "ProfileSwitchShowsAndHidesControls", ProfileSwitchShowsAndHidesControls },
        { "WheelZoomsWhenTouchIsEmulated", WheelZoomsWhenTouchIsEmulated },
    };

    for (const Test& test : tests)
    {
        const int failuresBefore = failures;
        test.run();
        std::printf("%s %s\n", failures == failuresBefore ? "PASS" : "FAIL", test.name);
    }

    std::printf("\n%d checks, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
