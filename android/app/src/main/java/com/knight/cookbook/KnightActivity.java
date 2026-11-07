package com.knight.cookbook;

import android.app.NativeActivity;
import android.content.res.Configuration;
import android.graphics.Rect;
import android.os.Build;
import android.os.Bundle;
import android.view.DisplayCutout;
import android.view.View;
import android.view.WindowInsets;
import android.view.WindowInsetsController;

import androidx.core.content.ContextCompat;
import androidx.core.util.Consumer;
import androidx.window.java.layout.WindowInfoTrackerCallbackAdapter;
import androidx.window.layout.DisplayFeature;
import androidx.window.layout.FoldingFeature;
import androidx.window.layout.WindowInfoTracker;
import androidx.window.layout.WindowLayoutInfo;

/**
 * Knight's Android activity: raylib's NativeActivity plus what native code
 * cannot get on its own. It keeps the game full screen (system bars hidden,
 * drawing under the camera cutout) and reports the safe area, density and
 * fold state to Knight's Display class whenever they change.
 *
 * Everything here runs on the UI thread. nativeOnDisplayChanged only stores
 * the values; Knight applies them on the game thread at the start of a frame.
 */
public class KnightActivity extends NativeActivity {
    // Values for nativeOnDisplayChanged; keep in sync with Display.cpp.
    private static final int FOLD_NOT_FOLDABLE = 0;
    private static final int FOLD_FLAT = 1;
    private static final int FOLD_HALF_OPENED = 2;
    private static final int HINGE_NONE = 0;
    private static final int HINGE_VERTICAL = 1;
    private static final int HINGE_HORIZONTAL = 2;

    static {
        // NativeActivity loads libmain itself, but only System.loadLibrary
        // lets Java find nativeOnDisplayChanged in it. Both share one copy.
        System.loadLibrary("main");
    }

    private static native void nativeOnDisplayChanged(
        int insetLeft, int insetTop, int insetRight, int insetBottom,
        float density, int fold, int hingeAxis,
        int hingeX, int hingeY, int hingeWidth, int hingeHeight,
        boolean hingeSeparates);

    private WindowInfoTrackerCallbackAdapter windowInfoTracker;
    private final Consumer<WindowLayoutInfo> layoutListener = this::onWindowLayoutChanged;

    // Latest fold state from WindowInfoTracker.
    private int fold = FOLD_NOT_FOLDABLE;
    private int hingeAxis = HINGE_NONE;
    private final Rect hinge = new Rect();
    private boolean hingeSeparates = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        if (Build.VERSION.SDK_INT >= 30) {
            getWindow().setDecorFitsSystemWindows(false);
        }
        hideSystemBars();

        View decor = getWindow().getDecorView();
        decor.setOnApplyWindowInsetsListener((view, insets) -> {
            reportDisplay(insets);
            return view.onApplyWindowInsets(insets);
        });

        windowInfoTracker = new WindowInfoTrackerCallbackAdapter(
            WindowInfoTracker.getOrCreate(this));
    }

    @Override
    protected void onStart() {
        super.onStart();
        windowInfoTracker.addWindowLayoutInfoListener(
            this, ContextCompat.getMainExecutor(this), layoutListener);
    }

    @Override
    protected void onStop() {
        windowInfoTracker.removeWindowLayoutInfoListener(layoutListener);
        super.onStop();
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        // The bars come back when another window takes focus (a dialog, the
        // notification shade); hide them again once the game has it back.
        if (hasFocus) {
            hideSystemBars();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        // Insets are laid out after the configuration change is applied.
        View decor = getWindow().getDecorView();
        decor.post(() -> reportDisplay(decor.getRootWindowInsets()));
    }

    private void onWindowLayoutChanged(WindowLayoutInfo info) {
        fold = FOLD_NOT_FOLDABLE;
        hingeAxis = HINGE_NONE;
        hinge.setEmpty();
        hingeSeparates = false;

        for (DisplayFeature feature : info.getDisplayFeatures()) {
            if (!(feature instanceof FoldingFeature)) {
                continue;
            }
            FoldingFeature folding = (FoldingFeature) feature;
            fold = folding.getState() == FoldingFeature.State.HALF_OPENED
                ? FOLD_HALF_OPENED : FOLD_FLAT;
            hingeAxis = folding.getOrientation() == FoldingFeature.Orientation.HORIZONTAL
                ? HINGE_HORIZONTAL : HINGE_VERTICAL;
            hinge.set(folding.getBounds());
            hingeSeparates = folding.isSeparating();
            break;
        }

        reportDisplay(getWindow().getDecorView().getRootWindowInsets());
    }

    private void hideSystemBars() {
        View decor = getWindow().getDecorView();
        if (Build.VERSION.SDK_INT >= 30) {
            WindowInsetsController controller = getWindow().getInsetsController();
            if (controller != null) {
                controller.hide(WindowInsets.Type.systemBars());
                controller.setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
            }
        } else {
            decor.setSystemUiVisibility(
                View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION);
        }
    }

    // Insets are in window pixels, the same pixels the game draws in.
    private void reportDisplay(WindowInsets insets) {
        int left = 0, top = 0, right = 0, bottom = 0;
        if (insets != null) {
            if (Build.VERSION.SDK_INT >= 30) {
                // Bars that are hidden report no inset; the cutout always does.
                android.graphics.Insets safe = insets.getInsets(
                    WindowInsets.Type.systemBars() | WindowInsets.Type.displayCutout());
                left = safe.left;
                top = safe.top;
                right = safe.right;
                bottom = safe.bottom;
            } else {
                left = insets.getSystemWindowInsetLeft();
                top = insets.getSystemWindowInsetTop();
                right = insets.getSystemWindowInsetRight();
                bottom = insets.getSystemWindowInsetBottom();
                if (Build.VERSION.SDK_INT >= 28) {
                    DisplayCutout cutout = insets.getDisplayCutout();
                    if (cutout != null) {
                        left = Math.max(left, cutout.getSafeInsetLeft());
                        top = Math.max(top, cutout.getSafeInsetTop());
                        right = Math.max(right, cutout.getSafeInsetRight());
                        bottom = Math.max(bottom, cutout.getSafeInsetBottom());
                    }
                }
            }
        }

        float density = getResources().getDisplayMetrics().density;
        nativeOnDisplayChanged(left, top, right, bottom, density, fold, hingeAxis,
            hinge.left, hinge.top, hinge.width(), hinge.height(), hingeSeparates);
    }
}
