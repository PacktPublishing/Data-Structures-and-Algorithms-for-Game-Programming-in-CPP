#include "Display.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

#if defined(PLATFORM_ANDROID)
#include <jni.h>
#include <mutex>
#endif

DisplayInfo Display::current;

namespace
{
    Rectangle Intersect(Rectangle a, Rectangle b)
    {
        const float left = std::max(a.x, b.x);
        const float top = std::max(a.y, b.y);
        const float right = std::min(a.x + a.width, b.x + b.width);
        const float bottom = std::min(a.y + a.height, b.y + b.height);
        if (right <= left || bottom <= top)
            return Rectangle{ 0, 0, 0, 0 };
        return Rectangle{ left, top, right - left, bottom - top };
    }

    bool SameRectangle(Rectangle a, Rectangle b)
    {
        return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
    }

#if defined(PLATFORM_ANDROID)
    std::mutex reportMutex;
    PlatformDisplayReport pendingReport;
#else
    // Desktop and web: developer fakes.
    int fakeFoldStep = 0;

#if !defined(PLATFORM_WEB)
    // KNIGHT_SAFE_AREA=left,top,right,bottom, read once.
    bool ReadFakeInsets(PlatformDisplayReport& report)
    {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4996) // getenv is fine for a read-only lookup
#endif
        const char* value = std::getenv("KNIGHT_SAFE_AREA");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
        int left = 0, top = 0, right = 0, bottom = 0;
        if (value == nullptr ||
            std::sscanf(value, "%d,%d,%d,%d", &left, &top, &right, &bottom) != 4)
            return false;
        report.insetLeft = left;
        report.insetTop = top;
        report.insetRight = right;
        report.insetBottom = bottom;
        return true;
    }
#endif

    PlatformDisplayReport DesktopReport(int width, int height)
    {
        PlatformDisplayReport report;
#if !defined(PLATFORM_WEB)
        static PlatformDisplayReport insets;
        static bool insetsRead = false;
        if (!insetsRead)
        {
            ReadFakeInsets(insets);
            insetsRead = true;
        }
        report = insets;
#endif
        const float w = static_cast<float>(width);
        const float h = static_cast<float>(height);
        switch (fakeFoldStep)
        {
        case 1: // Unfolded book-style foldable: a seamless vertical hinge.
            report.fold = FoldState::Flat;
            report.hingeAxis = HingeAxis::Vertical;
            report.hinge = Rectangle{ w * 0.5f, 0, 0, h };
            report.hingeSeparates = false;
            break;
        case 2: // Tabletop: half-opened with the fold across the middle.
            report.fold = FoldState::HalfOpened;
            report.hingeAxis = HingeAxis::Horizontal;
            report.hinge = Rectangle{ 0, h * 0.5f, w, 0 };
            report.hingeSeparates = true;
            break;
        default:
            break;
        }
        return report;
    }
#endif
}

Rectangle DisplayInfo::ControlArea() const
{
    if (hingeSeparates && hingeAxis == HingeAxis::Horizontal)
    {
        const float top = hinge.y + hinge.height;
        const Rectangle lowerPart = Intersect(safeArea,
            Rectangle{ 0, top, static_cast<float>(width), height - top });
        if (lowerPart.width > 0 && lowerPart.height > 0)
            return lowerPart;
    }
    return safeArea;
}

bool DisplayInfo::operator==(const DisplayInfo& other) const
{
    return width == other.width && height == other.height &&
        dpScale == other.dpScale &&
        SameRectangle(safeArea, other.safeArea) &&
        fold == other.fold && hingeAxis == other.hingeAxis &&
        SameRectangle(hinge, other.hinge) &&
        hingeSeparates == other.hingeSeparates;
}

DisplayInfo Display::Compose(int width, int height, float rendererDpScale,
    const PlatformDisplayReport& report)
{
    DisplayInfo info;
    info.width = std::max(width, 0);
    info.height = std::max(height, 0);
    info.dpScale = report.dpScale > 0.0f ? report.dpScale
        : rendererDpScale > 0.0f ? rendererDpScale : 1.0f;

    int left = std::max(report.insetLeft, 0);
    int top = std::max(report.insetTop, 0);
    int right = std::max(report.insetRight, 0);
    int bottom = std::max(report.insetBottom, 0);
    if (left + right >= info.width || top + bottom >= info.height)
        left = top = right = bottom = 0;
    info.safeArea = Rectangle{
        static_cast<float>(left), static_cast<float>(top),
        static_cast<float>(info.width - left - right),
        static_cast<float>(info.height - top - bottom)
    };

    info.fold = report.fold;
    info.hingeAxis = report.hingeAxis;
    info.hinge = report.hinge;
    info.hingeSeparates = report.hingeSeparates;
    return info;
}

bool Display::Poll(DisplayInfo& next)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    PlatformDisplayReport report;
#if defined(PLATFORM_ANDROID)
    {
        std::lock_guard<std::mutex> lock(reportMutex);
        report = pendingReport;
    }
#else
    report = DesktopReport(width, height);
#endif
    next = Compose(width, height, GetWindowScaleDPI().x, report);
    return next != current;
}

void Display::PostPlatformReport(const PlatformDisplayReport& report)
{
#if defined(PLATFORM_ANDROID)
    std::lock_guard<std::mutex> lock(reportMutex);
    pendingReport = report;
#else
    (void)report;
#endif
}

void Display::CycleFakeFold()
{
#if !defined(PLATFORM_ANDROID)
    fakeFoldStep = (fakeFoldStep + 1) % 3;
#endif
}

#if defined(PLATFORM_ANDROID)
// Called by KnightActivity (android/app/src/main/java/com/knight/cookbook)
// on the UI thread whenever insets, density or the fold state change.
// Keep the signature in sync with nativeOnDisplayChanged there.
extern "C" JNIEXPORT void JNICALL
Java_com_knight_cookbook_KnightActivity_nativeOnDisplayChanged(JNIEnv*, jclass,
    jint insetLeft, jint insetTop, jint insetRight, jint insetBottom,
    jfloat density, jint fold, jint hingeAxis,
    jint hingeX, jint hingeY, jint hingeWidth, jint hingeHeight,
    jboolean hingeSeparates)
{
    PlatformDisplayReport report;
    report.insetLeft = insetLeft;
    report.insetTop = insetTop;
    report.insetRight = insetRight;
    report.insetBottom = insetBottom;
    report.dpScale = density;
    report.fold = fold == 1 ? FoldState::Flat
        : fold == 2 ? FoldState::HalfOpened : FoldState::NotFoldable;
    report.hingeAxis = hingeAxis == 1 ? HingeAxis::Vertical
        : hingeAxis == 2 ? HingeAxis::Horizontal : HingeAxis::None;
    report.hinge = Rectangle{
        static_cast<float>(hingeX), static_cast<float>(hingeY),
        static_cast<float>(hingeWidth), static_cast<float>(hingeHeight)
    };
    report.hingeSeparates = hingeSeparates == JNI_TRUE;
    Display::PostPlatformReport(report);
}
#endif
