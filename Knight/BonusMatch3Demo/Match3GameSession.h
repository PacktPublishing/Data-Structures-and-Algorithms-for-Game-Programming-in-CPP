#pragma once

#include "Knight.h"
#include "Match3GameConfig.h"
#include "Match3Board.h"

#include "RandomGenerator.h"
#include "ParticlePool.h"

#include <algorithm>
#include <unordered_map>

#include "rlgl.h"

// A lightweight projectile used by Color Bomb "missile mode".
struct Missile 
{
    int tr, tc;            // target tile
    Vector2 start, end;    // world-space points
    float delay;           // launch time offset
    float travel;          // duration
    float t;               // 0..1
    bool  hit;             // impact done
};

class Match3GameSession 
{
public:

    Match3GameSession()
    {
        cam = Camera2D();
        cam.zoom = 1.0f;
        state = Idle;
        clearMode = CM_Mask;
        dragStart = Vec2i{ -1,-1 };
        dragStartPos = { 0,0 };
        dragging = false;
        clearAnimTime = 0.f;
        waveTime = 0.f;
        waveOriginR = -1;
        waveOriginC = -1;
        waveColor = -1;

        for (int r = 0; r < ROWS; ++r)
            for (int c = 0; c < COLS; ++c)
                explodeStart[r][c] = -1.f;

        midTriggerArmed = false;
        midTriggerTime = 0.f;
        shakeT = 0.f;
        shakeMag = SHAKE_MAG;
        shakeDur = SHAKE_DURATION;
        slowmoT = 0.f;
        flashT = 0.f;
        lastInteraction = (float)GetTime();
        hintA = Vec2i{ -1,-1 };
        hintB = Vec2i{ -1,-1 };
        hintScore = 0;
    }

    bool Create();
    void Update(float dtReal);
    void Draw();

protected:
    Board bd;
    Camera2D cam;

    GameState state;
    ClearMode clearMode;

    Swap swap;
    Vec2i dragStart; 
    Vector2 dragStartPos; 
    bool dragging;

    // Clear-anim (mask)
    std::vector<std::vector<bool>> mark;
    std::vector<std::vector<bool>> clearMask;
    std::vector<Board::SpawnPlan> spawnPlans;
    float clearAnimTime;

    // Color Bomb missile mode
    float waveTime;
    int   waveOriginR, waveOriginC, waveColor;
    float explodeStart[ROWS][COLS]; // per-tile
    std::vector<Missile> missiles;
    bool  midTriggerArmed;
    float midTriggerTime;

    // FX
    float shakeT, shakeMag, shakeDur;
    float slowmoT, flashT;

    // Hint
    float lastInteraction;
    Vec2i hintA, hintB; int hintScore;

    // Rendering 
    float dtReal = 0;
    float dt = 0;
    float now = 0;
    float hintTime = 0.0f;

    std::unordered_map<int, Texture2D> chipTextures;
    Texture2D bkgd = { 0 };
    Texture2D missile = { 0 };

private:

    bool AnimateOffsetsTowardZero(float dt);

    void StartClearMask(const std::vector<std::vector<bool>>& m, const std::vector<Board::SpawnPlan>& sp);

    float ComputeLargestClusterMedianDelay(int targetColor);

    void StartColorBombWave(int originR, int originC, int color);

    void FindBestMoveAndSetHint();

    void DrawMissile(const Missile& m, float tt, Texture2D missileTex);

    void MyDrawRing(float progress);
};


//End of Match3GameSession.h
