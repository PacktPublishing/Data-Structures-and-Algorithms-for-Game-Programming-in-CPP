#pragma once

#include "raylib.h"

// Posture of a foldable screen. NotFoldable on every other device.
enum class FoldState
{
    NotFoldable,
    Flat,           // Unfolded; a hinge may still cross the screen
    HalfOpened      // Folded part way, like a laptop or a book
};

// Direction of the line a hinge draws across the screen.
enum class HingeAxis
{
    None,
    Vertical,       // Splits the screen into left and right
    Horizontal      // Splits the screen into top and bottom
};

// Everything Knight knows about the screen, in pixels of the drawable area
// (on Android the whole physical screen).
struct DisplayInfo
{
    int width = 0;
    int height = 0;
    // Pixels per density-independent pixel (1 dp = 1 px at 160 dpi).
    float dpScale = 1.0f;
    // Part of the screen clear of notches, rounded corners and system bars.
    // The scene still draws everywhere; on-screen controls and HUD text
    // belong inside this.
    Rectangle safeArea = { 0, 0, 0, 0 };
    FoldState fold = FoldState::NotFoldable;
    HingeAxis hingeAxis = HingeAxis::None;
    // Where the hinge crosses the screen; zero width or height for a hinge
    // with no gap, empty when there is no hinge.
    Rectangle hinge = { 0, 0, 0, 0 };
    // True when content cannot span the hinge: the screen is half-opened or
    // the hinge is a physical gap between two panels.
    bool hingeSeparates = false;

    // Where on-screen controls go: the safe area, minus the part above a
    // horizontal hinge that separates the screen (tabletop posture), so
    // controls sit on the half lying flat.
    Rectangle ControlArea() const;

    bool operator==(const DisplayInfo& other) const;
    bool operator!=(const DisplayInfo& other) const { return !(*this == other); }
};

// What the platform reports besides the window size, which always comes from
// raylib. Android fills it from Java; the desktop fakes below fill it too.
struct PlatformDisplayReport
{
    // Insets from each screen edge that the safe area excludes, in pixels.
    int insetLeft = 0;
    int insetTop = 0;
    int insetRight = 0;
    int insetBottom = 0;
    // 0 when unknown: Display then asks raylib.
    float dpScale = 0.0f;
    FoldState fold = FoldState::NotFoldable;
    HingeAxis hingeAxis = HingeAxis::None;
    Rectangle hinge = { 0, 0, 0, 0 };
    bool hingeSeparates = false;
};

// The screen as Knight last saw it: size, density, safe area and fold state.
// Read it from anywhere. Knight updates it once per frame on the game thread,
// before input and before Update(), and calls Knight::OnDisplayChange() when
// something changed, so values never change in the middle of a frame.
//
// On desktop and web the size follows the window or canvas and the safe area
// is the whole window. Two developer fakes let you see phone layouts there:
//   KNIGHT_SAFE_AREA=left,top,right,bottom   insets in pixels (env variable)
//   F5                                       cycle fake fold states
class Display
{
public:
    static const DisplayInfo& Info() { return current; }
    static int Width() { return current.width; }
    static int Height() { return current.height; }
    static float DpScale() { return current.dpScale; }
    static Rectangle SafeArea() { return current.safeArea; }
    static Rectangle ControlArea() { return current.ControlArea(); }
    static FoldState Fold() { return current.fold; }
    static HingeAxis Hinge() { return current.hingeAxis; }
    static Rectangle HingeBounds() { return current.hinge; }

    // Called by Knight each frame. Reads the window size from raylib and
    // the latest platform report, and returns true when the result differs
    // from Info(). It does not change Info(); Knight applies it with Apply().
    static bool Poll(DisplayInfo& next);
    static void Apply(const DisplayInfo& info) { current = info; }

    // Builds the info for a window of the given size and a platform report.
    // Insets that would leave no safe area are ignored.
    static DisplayInfo Compose(int width, int height, float rendererDpScale,
        const PlatformDisplayReport& report);

    // Thread safe: Android's UI thread calls this from Java. The report is
    // picked up by the next Poll().
    static void PostPlatformReport(const PlatformDisplayReport& report);

    // Desktop developer fake: steps through not foldable, flat with a
    // vertical hinge and half-opened with a horizontal hinge.
    static void CycleFakeFold();

private:
    static DisplayInfo current;
};
