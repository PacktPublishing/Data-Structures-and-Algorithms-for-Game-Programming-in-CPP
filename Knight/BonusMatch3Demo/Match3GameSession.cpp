#include "Match3GameSession.h"

ParticlePool gPool;
RNGStreams gRng;

//Loads textures (chips, specials, background, missile), seeds the board with a valid layout, and precomputes a hint.
bool Match3GameSession::Create()
{
    //load chip textures
    chipTextures[0] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-blue.png").c_str());
    chipTextures[1] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-red.png").c_str());
    chipTextures[2] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-orange.png").c_str());
    chipTextures[3] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-green.png").c_str());
    chipTextures[4] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-purple.png").c_str());

    chipTextures[10] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-crossbomb.png").c_str());
    chipTextures[11] = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/chip-rocketbomb.png").c_str());

    bkgd = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/halloween.png").c_str());
    missile = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/flash00.png").c_str());

    bd.FillNoMatches();
    bd.FindAnyMoveHeuristic(hintA, hintB, &hintScore);

	return true;
}

//Main state machine: handles input (drag/click/swap), swap animation, revert if no match, clear phase (mask or ?missile wave? for color bomb), 
// collapse/refill cycles, shuffle when stuck, camera shake/flash/slow-mo timing, particle system update, and idle hint refresh.
void Match3GameSession::Update(float dtReal)
{
    dt = (slowmoT > 0.f) ? (dtReal * SLOWMO_SCALE) : dtReal;
    now = (float)GetTime();
    if (slowmoT > 0.f)
        slowmoT -= dtReal;
    if (flashT > 0.f)
        flashT -= dtReal;

    // Input when idle
    if (state == Idle) {

        Vector2 mp = GetMousePosition();

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            dragStart = screenToCell(mp);
            if (bd.InBounds(dragStart.r, dragStart.c))
            {
                dragging = true;
                dragStartPos = mp;
                lastInteraction = now;
            }
        }

        if (dragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
        {
            Vector2 rel = Vector2{ mp.x - dragStartPos.x, mp.y - dragStartPos.y };
            Vec2i end = screenToCell(mp);

            const float CLICK_THRESHOLD = 6.f;
            if (fabsf(rel.x) < CLICK_THRESHOLD && fabsf(rel.y) < CLICK_THRESHOLD && bd.InBounds(dragStart.r, dragStart.c))
            {
                int sp = bd.special[dragStart.r][dragStart.c];

                if (sp == SP_CrossBomb || sp == SP_ColorBomb)
                {

                    if (sp == SP_CrossBomb)
                    {
                        std::vector<std::vector<bool>> m(ROWS, std::vector<bool>(COLS, false));
                        std::vector<Board::SpawnPlan> none;
                        int r = dragStart.r, c = dragStart.c;

                        m[r][c] = true;
                        if (bd.InBounds(r - 1, c)) m[r - 1][c] = true;
                        if (bd.InBounds(r + 1, c)) m[r + 1][c] = true;
                        if (bd.InBounds(r, c - 1)) m[r][c - 1] = true;
                        if (bd.InBounds(r, c + 1)) m[r][c + 1] = true;
                        StartClearMask(m, none);
                    }
                    else
                    {
                        int r = dragStart.r, c = dragStart.c, clr = bd.cell[r][c];
                        StartColorBombWave(r, c, clr);
                    }
                    dragging = false;

                    goto SKIP_DRAG_SWAP;
                }
            }

            int dr = 0, dc = 0;
            if (fabsf(rel.x) > fabsf(rel.y))
                dc = (rel.x > 0) ? 1 : -1; else dr = (rel.y > 0) ? 1 : -1;

            if (dr == 0 && dc == 0 && bd.InBounds(end.r, end.c))
            {
                if (abs(end.r - dragStart.r) + abs(end.c - dragStart.c) == 1)
                {
                    dr = end.r - dragStart.r;
                    dc = end.c - dragStart.c;
                }
            }

            {
                Vec2i B{ dragStart.r + dr, dragStart.c + dc };
                if (bd.InBounds(dragStart.r, dragStart.c) && bd.InBounds(B.r, B.c) && (abs(dr) + abs(dc) == 1)) 
                {
                    swap.a = dragStart; 
                    swap.b = B; 
                    swap.t = 0.f; 
                    state = Swapping;
                    bd.offX[swap.a.r][swap.a.c] = bd.offY[swap.a.r][swap.a.c] = 0;
                    bd.offX[swap.b.r][swap.b.c] = bd.offY[swap.b.r][swap.b.c] = 0;
                }
            }
            dragging = false;
        }
    }
SKIP_DRAG_SWAP:;

    // FSM
    switch (state)
    {
    case Idle:
    {
        for (int r = 0; r < ROWS; ++r) 
            for (int c = 0; c < COLS; ++c)
                bd.highlight[r][c] = std::max(0.f, bd.highlight[r][c] - dt * 3.f);

        if (now - lastInteraction > IDLE_HINT_SEC)
        {
            static float acc = 0;
            acc += dtReal;
            if (acc > 1.f)
            {
                acc = 0; 
                FindBestMoveAndSetHint();
            }
        }
        else
        {
            hintA = Vec2i{ -1,-1 };
            hintB = Vec2i{ -1,-1 };
        }
    }
    break;

    case Swapping:
    {
        swap.t += dt;
        {
            float p = std::min(1.f, swap.t / SWAP_TIME);
            int dr = swap.b.r - swap.a.r, dc = swap.b.c - swap.a.c;

            bd.offX[swap.a.r][swap.a.c] = dc * p * CELL;
            bd.offY[swap.a.r][swap.a.c] = dr * p * CELL;
            bd.offX[swap.b.r][swap.b.c] = -dc * p * CELL;
            bd.offY[swap.b.r][swap.b.c] = -dr * p * CELL;

            if (p >= 1.f)
            {
                std::swap(bd.cell[swap.a.r][swap.a.c], bd.cell[swap.b.r][swap.b.c]);
                std::swap(bd.special[swap.a.r][swap.a.c], bd.special[swap.b.r][swap.b.c]);
                bd.offX[swap.a.r][swap.a.c] = bd.offY[swap.a.r][swap.a.c] = 0;
                bd.offX[swap.b.r][swap.b.c] = bd.offY[swap.b.r][swap.b.c] = 0;

                std::vector<Board::SpawnPlan> sp; 
                
                int mcount = bd.FindMatchesWithSpawns(mark, swap, sp);
                if (mcount > 0)
                {
                    for (size_t i = 0; i < sp.size(); ++i)
                    {
                        Board::SpawnPlan s = sp[i];
                        if (mark[s.r][s.c])
                            mark[s.r][s.c] = false;
                    }
                    StartClearMask(mark, sp);
                }
                else
                {
                    swap.t = 0;
                    state = Reverting;
                }
                lastInteraction = now;
            }
        }
    }
    break;

    case Reverting:
    {
        swap.t += dt;
        {
            float p = std::min(1.f, swap.t / SWAP_TIME);
            int dr = swap.b.r - swap.a.r, dc = swap.b.c - swap.a.c;

            bd.offX[swap.a.r][swap.a.c] = dc * (1.f - p) * CELL;
            bd.offY[swap.a.r][swap.a.c] = dr * (1.f - p) * CELL;
            bd.offX[swap.b.r][swap.b.c] = -dc * (1.f - p) * CELL;
            bd.offY[swap.b.r][swap.b.c] = -dr * (1.f - p) * CELL;

            if (p >= 1.f)
            {
                bd.offX[swap.a.r][swap.a.c] = bd.offY[swap.a.r][swap.a.c] = 0;
                bd.offX[swap.b.r][swap.b.c] = bd.offY[swap.b.r][swap.b.c] = 0;
                swap.reset(); state = Idle;
            }
        }
    }
    break;

    case Clearing:
    {
        if (clearMode == CM_Mask)
        {
            clearAnimTime += dt;
            if (clearAnimTime >= CLEAR_ANIM_TIME)
            {
                bd.ClearByMask(clearMask, spawnPlans);
                if (bd.CollapseExisting())
                    state = FallingCollapse;
                else
                {
                    bd.RefillNew();
                    state = FallingRefill;
                }
            }
        }
        else
        {   // CM_Wave (Color Bomb missiles)
            waveTime += dt;

            if (midTriggerArmed && waveTime >= midTriggerTime)
            {
                midTriggerArmed = false;
                slowmoT = std::max(slowmoT, SECOND_SLOWMO_DURATION);
                flashT = std::max(flashT, SECOND_FLASH_DURATION);
                shakeMag = SECOND_SHAKE_MAG; shakeDur = SECOND_SHAKE_DURATION; shakeT = std::max(shakeT, shakeDur);
            }

            bool allHit = true;
            for (size_t i = 0; i < missiles.size(); ++i)
            {
                Missile& m = missiles[i];
                if (m.hit) continue;
                float lt = waveTime - m.delay;
                if (lt < 0.f)
                {
                    allHit = false;
                    continue;
                }

                float t = clamp01(lt / m.travel);
                m.t = t;
                if (t < 1.f)
                {
                    allHit = false;
                }
                else
                {
                    if (!m.hit)
                    {
                        m.hit = true;
                        Rectangle rc = cellRect(m.tr, m.tc);
                        Vector2 center{ rc.x + CELL * 0.5f, rc.y + CELL * 0.5f };
                        Color col = CHIP_COLORS[waveColor]; col.a = 230;
                        gPool.SpawnBurst(center, col, BURST_PARTICLES, BURST_SPEED_MIN, BURST_SPEED_MAX, BURST_SIZE_MIN, BURST_SIZE_MAX, BURST_LIFETIME);
                        explodeStart[m.tr][m.tc] = waveTime;
                    }
                }
            }

            bool allExplodedGone = true;

            for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c)
            {
                if (explodeStart[r][c] >= 0.f)
                {
                    float local = (waveTime - explodeStart[r][c]) / CLEAR_ANIM_TIME;
                    if (local < 1.f)
                        allExplodedGone = false;
                }
            }

            if (allHit && allExplodedGone)
            {
                std::vector<std::vector<bool>> m(ROWS, std::vector<bool>(COLS, false));
                for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c)
                    if (bd.cell[r][c] == waveColor)
                        m[r][c] = true;
                std::vector<Board::SpawnPlan> none;
                bd.ClearByMask(m, none);
                if (bd.CollapseExisting())
                    state = FallingCollapse;
                else
                {
                    bd.RefillNew();
                    state = FallingRefill;
                }
            }
        }
    }
    break;

    case FallingCollapse:
    {
        bool anim = AnimateOffsetsTowardZero(dt);
        if (!anim) {
            if (bd.AnyEmpty())
            {
                bd.RefillNew();
                state = FallingRefill;
            }
            else
            {
                std::vector<Board::SpawnPlan> sp;
                int mcount = bd.FindMatchesWithSpawns(mark, Swap(), sp);

                if (mcount > 0)
                {
                    for (size_t i = 0; i < sp.size(); ++i)
                    {
                        Board::SpawnPlan s = sp[i];
                        if (mark[s.r][s.c])
                            mark[s.r][s.c] = false;
                    }
                    StartClearMask(mark, sp);
                }
                else
                {
                    Vec2i A, B;
                    if (!bd.FindAnyMoveHeuristic(A, B))
                        state = Shuffling;
                    else
                    {
                        hintA = A;
                        hintB = B;
                        state = Idle;
                    }
                }
            }
        }
    }
    break;

    case FallingRefill:
    {
        bool anim = AnimateOffsetsTowardZero(dt);
        if (!anim)
        {
            std::vector<Board::SpawnPlan> sp;
            int mcount = bd.FindMatchesWithSpawns(mark, Swap(), sp);
            if (mcount > 0)
            {
                for (size_t i = 0; i < sp.size(); ++i)
                {
                    Board::SpawnPlan s = sp[i];
                    if (mark[s.r][s.c])
                        mark[s.r][s.c] = false;
                }
                StartClearMask(mark, sp);
            }
            else
            {
                Vec2i A, B;
                if (!bd.FindAnyMoveHeuristic(A, B))
                    state = Shuffling;
                else
                {
                    hintA = A;
                    hintB = B;
                    state = Idle;
                }
            }
        }
    }
    break;

    case Shuffling:
    {
        bd.Shuffle();

        std::vector<Board::SpawnPlan> sp;

        int mcount = bd.FindMatchesWithSpawns(mark, Swap(), sp);

        if (mcount > 0)
        {
            for (size_t i = 0; i < sp.size(); ++i)
            {
                Board::SpawnPlan s = sp[i];
                if (mark[s.r][s.c])
                    mark[s.r][s.c] = false;
            }
            StartClearMask(mark, sp);
        }
        else
        {
            Vec2i A, B;
            bd.FindAnyMoveHeuristic(A, B);
            hintA = A;
            hintB = B;
            state = Idle;
        }
    }
    break;
    }

    // Particles
    gPool.Update((slowmoT > 0.f) ? (dtReal * SLOWMO_SCALE) : dtReal);

    // Shake
    if (shakeT > 0.f)
    {
        shakeT -= dtReal;
        float tleft = clamp01(shakeT / shakeDur);
        float amp = shakeMag * (0.5f + 0.5f * tleft);
        float tt = (float)GetTime();
        cam.offset.x = sinf(tt * 2 * PI * SHAKE_FREQ) * amp;
        cam.offset.y = cosf((tt + 0.37f) * 2 * PI * (SHAKE_FREQ * 0.85f)) * amp;
    }
    else
    {
        cam.offset = Vector2{ 0,0 };
        shakeMag = SHAKE_MAG;
        shakeDur = SHAKE_DURATION;
    }

}

//Renders background, grid, in-progress clear effects (mask shrink or wave ring), chips (with highlights, 
// specials, scaling/alpha fades), drag & hint outlines, particles, missiles with trails, and optional screen flash.
void Match3GameSession::Draw()
{
    BeginMode2D(cam);

    DrawTexture(bkgd, 0, 0, WHITE);

    // Grid
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            DrawRectangleRoundedLinesEx(cellRect(r, c), 0.2f, 4, 2, Color{ 60,65,72,200 });

    // Optional ring cue
    if (state == Clearing && clearMode == CM_Wave && waveOriginR >= 0)
    {
        float minD = 1e9f, maxD = 0.f;

        for (size_t i = 0; i < missiles.size(); ++i)
        {
            const Missile& m = missiles[i];
            if (m.delay < minD)
                minD = m.delay;
            if (m.delay + m.travel > maxD)
                maxD = m.delay + m.travel;
        }
        float prog = 0.f;
        if (maxD > 0)
            prog = clamp01((waveTime - minD) / (maxD - minD + 1e-6f));
        MyDrawRing(prog);
    }

    // Chips (with mask/wave shrink)
    float tMask = (state == Clearing && clearMode == CM_Mask) ? std::min(1.f, clearAnimTime / CLEAR_ANIM_TIME) : 0.f;
    float eMask = EaseOutCubic(tMask);

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (bd.cell[r][c] < 0)
                continue;

            float scale = 1.f; unsigned char alpha = 255;

            bool isMaskClear = (state == Clearing && clearMode == CM_Mask &&
                r < (int)clearMask.size() && c < (int)clearMask[r].size() && clearMask[r][c]);

            if (isMaskClear)
            {
                float e = eMask; scale = 1.f - e; alpha = (unsigned char)(255 * (1.f - e));
            }
            else
                if (state == Clearing && clearMode == CM_Wave && explodeStart[r][c] >= 0.f)
                {
                    float local = (waveTime - explodeStart[r][c]) / CLEAR_ANIM_TIME;
                    if (local > 0.f)
                    {
                        float e = EaseOutCubic(local < 1.f ? local : 1.f);
                        scale = 1.f - e; alpha = (unsigned char)(255 * (1.f - e));
                    }
                }

            Rectangle rc = cellRect(r, c);
            float x = rc.x + bd.offX[r][c], y = rc.y + bd.offY[r][c];
            float w = (CELL - 12) * scale, h = (CELL - 12) * scale;
            float cx = x + 6 + (CELL - 12) / 2.f, cy = y + 6 + (CELL - 12) / 2.f;

            Color base = CHIP_COLORS[bd.cell[r][c]];
            if (bd.highlight[r][c] > 0.f)
            {
                float a = bd.highlight[r][c];
                base = Color{
                    (unsigned char)std::min(255.f, base.r + 100 * a),
                    (unsigned char)std::min(255.f, base.g + 100 * a),
                    (unsigned char)std::min(255.f, base.b + 100 * a),
                    alpha };
            }
            else
                base.a = alpha;

            DrawRectangleRounded(Rectangle{ cx - w / 2.f, cy - h / 2.f, w, h }, 0.25f, 6, base);

            DrawRectangleRoundedLinesEx(Rectangle{ cx - w / 2.f, cy - h / 2.f, w, h }, 0.25f, 6, 3, Fade(WHITE, (alpha / 255.f) * 0.5f));

            hintTime += dtReal * 0.1f;
            if (hintTime >= PI)
                hintTime = 0.0f;
            float hw = w + sinf(hintTime) * 0.2f * w;
            float hh = h + sinf(hintTime) * 0.2f * h;

            switch (bd.special[r][c])
            {
            case SP_CrossBomb:
                DrawTexturePro(
                    chipTextures[10],
                    Rectangle{ 0,0,(float)chipTextures[10].width,(float)chipTextures[10].height },
                    Rectangle{ cx - w / 2.f, cy - h / 2.f, hw, hh },
                    Vector2{ 0,0 },
                    0.f,
                    base
                );
                break;
            case SP_ColorBomb:
                DrawTexturePro(
                    chipTextures[11],
                    Rectangle{ 0,0,(float)chipTextures[11].width,(float)chipTextures[11].height },
                    Rectangle{ cx - w / 2.f, cy - h / 2.f, hw, hh },
                    Vector2{ 0,0 },
                    0.f,
                    base
                );
                break;
            default:
                DrawTexturePro(
                    chipTextures[bd.cell[r][c]],
                    Rectangle{ 0,0,(float)chipTextures[bd.cell[r][c]].width,(float)chipTextures[bd.cell[r][c]].height },
                    Rectangle{ cx - w / 2.f, cy - h / 2.f, w, h },
                    Vector2{ 0,0 },
                    0.f,
                    Fade(WHITE, alpha / 255.f)
                );
                break;
            }

            DrawRectangleRoundedLinesEx(Rectangle{ cx - w / 2.f, cy - h / 2.f, w, h }, 0.25f, 6, 2, Fade(WHITE, (alpha / 255.f) * 0.15f));

        }
    }

    rlDisableDepthTest();

    // Drag highlight
    if (dragging && bd.InBounds(dragStart.r, dragStart.c))
        DrawRectangleRoundedLinesEx(cellRect(dragStart.r, dragStart.c), 0.3f, 6, 3, Color{ 255,255,255,200 });

    // Hint
    if (bd.InBounds(hintA.r, hintA.c) && bd.InBounds(hintB.r, hintB.c))
    {
        float phase = fmodf((float)now, HINT_BLINK_PERIOD) / HINT_BLINK_PERIOD;
        float a = 0.6f * (0.5f + 0.5f * sinf(phase * 2 * PI));
        Color hc = Color{ 255,255,0,(unsigned char)(a * 255) };
        DrawRectangleRoundedLinesEx(cellRect(hintA.r, hintA.c), 0.35f, 6, 4, hc);
        DrawRectangleRoundedLinesEx(cellRect(hintB.r, hintB.c), 0.35f, 6, 4, hc);
    }

    // Particles
    gPool.Draw();

    // Missiles on top
    if (state == Clearing && clearMode == CM_Wave)
    {
        for (size_t i = 0; i < missiles.size(); ++i)
        {
            const Missile& m = missiles[i];
            float lt = waveTime - m.delay;
            if (lt < 0.f)
                continue;
            float t = clamp01(lt / m.travel);
            DrawMissile(m, EaseInOutCubic(t), missile);
        }
    }

    rlEnableDepthTest();

    // Screen flash overlay
    if (flashT > 0.f)
    {
        float k = clamp01(flashT / FLASH_DURATION);
        float a = EaseOutCubic(k);
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 255,255,255,(unsigned char)(255 * (0.9f * a)) });
        EndBlendMode();
    }

    EndMode2D();
}

//Eases each tile?s offX/offY toward 0 at a fixed fall speed; returns whether any tile is still animating.
bool Match3GameSession::AnimateOffsetsTowardZero(float dt)
{
    bool active = false;

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (fabsf(bd.offY[r][c]) > 0.01f)
            {
                float dir = (bd.offY[r][c] > 0) ? -1.f : 1.f;
                float d = dir * FALL_SPEED * dt;

                if (fabsf(d) > fabsf(bd.offY[r][c]))
                    bd.offY[r][c] = 0;
                else 
                    bd.offY[r][c] += d;
                active = true;
            }

            if (fabsf(bd.offX[r][c]) > 0.01f)
            {
                float dir = (bd.offX[r][c] > 0) ? -1.f : 1.f;
                float d = dir * FALL_SPEED * dt;

                if (fabsf(d) > fabsf(bd.offX[r][c]))
                    bd.offX[r][c] = 0;
                else 
                    bd.offX[r][c] += d;
                active = true;
            }
        }
    }

    return active;
}

//Enters mask-clear mode, records mask/spawns, fires per-tile particle bursts, and starts the clear timer.
void Match3GameSession::StartClearMask(const std::vector<std::vector<bool>>& m, const std::vector<Board::SpawnPlan>& sp)
{
    clearMode = CM_Mask; clearMask = m;
    spawnPlans = sp;
    clearAnimTime = 0.f;
    state = Clearing;

    // particle bursts
    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (r < (int)clearMask.size() && c < (int)clearMask[r].size() && clearMask[r][c] && bd.cell[r][c] >= 0)
            {
                Rectangle rc = cellRect(r, c);
                Vector2 center{ rc.x + CELL * 0.5f, rc.y + CELL * 0.5f };
                Color col = CHIP_COLORS[bd.cell[r][c]]; col.a = 220;
                gPool.SpawnBurst(center, col, BURST_PARTICLES, BURST_SPEED_MIN, BURST_SPEED_MAX, BURST_SIZE_MIN, BURST_SIZE_MAX, BURST_LIFETIME);
            }
        }
    }
}

//Finds the largest 4-connected cluster of the target color and computes the median missile delay (distance-based) within that cluster to time mid-sequence FX.
float Match3GameSession::ComputeLargestClusterMedianDelay(int targetColor)
{
    bool isColor[ROWS][COLS]; for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c) isColor[r][c] = (bd.cell[r][c] == targetColor);
    bool visited[ROWS][COLS]; for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c) visited[r][c] = false;

    const int dr[4] = { -1,1,0,0 }, dc[4] = { 0,0,-1,1 };
    size_t bestCount = 0; std::vector<float> bestDel;

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (!isColor[r][c] || visited[r][c])
                continue;

            std::vector<Vec2i> comp; comp.push_back(Vec2i{ r,c }); visited[r][c] = true;

            for (size_t i = 0; i < comp.size(); ++i)
            {
                Vec2i v = comp[i];
                for (int k = 0; k < 4; ++k)
                {
                    int nr = v.r + dr[k], nc = v.c + dc[k];
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS && isColor[nr][nc] && !visited[nr][nc])
                    {
                        visited[nr][nc] = true; comp.push_back(Vec2i{ nr,nc });
                    }
                }
            }

            std::vector<float> compDel;
            for (size_t i = 0; i < comp.size(); ++i)
            {
                Vec2i v = comp[i];
                float dx = float(v.c - waveOriginC), dy = float(v.r - waveOriginR);
                float dist = sqrtf(dx * dx + dy * dy);
                compDel.push_back(dist * WAVE_DELAY_PER_CELL);
            }

            if (compDel.size() > bestCount)
            {
                bestCount = compDel.size();
                bestDel = compDel;
            }
        }

    }

    if (bestDel.empty())
        return 0.f;

    std::sort(bestDel.begin(), bestDel.end());

    return bestDel[bestDel.size() / 2];
}

//Configures ?missile wave? clear: schedules a missile to each tile of that color with radial delays, 
// arms mid-trigger FX (slow-mo/flash/shake), and resets explosion timers.
void Match3GameSession::StartColorBombWave(int originR, int originC, int color)
{
    clearMode = CM_Wave; state = Clearing;
    waveTime = 0.f;
    waveOriginR = originR;
    waveOriginC = originC;
    waveColor = color;
    missiles.clear();

    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            explodeStart[r][c] = -1.f;

    Vector2 startPos = { PADDING + originC * CELL + CELL * 0.5f + offsetX, PADDING + originR * CELL + CELL * 0.5f };

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (bd.cell[r][c] == color)
            {
                Vector2 endPos = { PADDING + c * CELL + CELL * 0.5f + offsetX, PADDING + r * CELL + CELL * 0.5f };
                float dx = float(c - originC), dy = float(r - originR);
                float dist = sqrtf(dx * dx + dy * dy);
                Missile m;
                m.tr = r;
                m.tc = c;
                m.start = startPos;
                m.end = endPos;
                m.delay = dist * WAVE_DELAY_PER_CELL;
                m.travel = MISSILE_TRAVEL_TIME;
                m.t = 0.f;
                m.hit = false;
                missiles.push_back(m);
            }
        }
    }

    midTriggerTime = ComputeLargestClusterMedianDelay(color);
    midTriggerArmed = (midTriggerTime > 0.f);

    // Opening FX
    shakeMag = SHAKE_MAG; shakeDur = SHAKE_DURATION; shakeT = shakeDur;
    slowmoT = SLOWMO_DURATION; flashT = FLASH_DURATION;
}

//Queries the board for the best heuristic move and caches it for the blinking hint.
void Match3GameSession::FindBestMoveAndSetHint()
{
    bd.FindAnyMoveHeuristic(hintA, hintB, &hintScore);
}

void Match3GameSession::DrawMissile(const Missile& m, float tt, Texture2D missileTex)
{
    Vector2 p0 = m.start, p1 = m.end;
    Vector2 pos = { p0.x + (p1.x - p0.x) * tt, p0.y + (p1.y - p0.y) * tt };
    pos.y -= sinf(tt * PI) * MISSILE_ARC_HEIGHT;

    Color trailC = CHIP_COLORS[waveColor]; trailC.a = 180;
    for (int i = 0; i < TRAIL_PARTICLES_PER_STEP; ++i)
    {
        gPool.SpawnBurst(pos, trailC, 30, TRAIL_SPEED_MIN, TRAIL_SPEED_MAX, TRAIL_SIZE_MIN, TRAIL_SIZE_MAX, TRAIL_LIFETIME);
    }

    BeginBlendMode(BLEND_ADDITIVE);
    Color head = CHIP_COLORS[waveColor];

    //The texture is drawn scaled down to 25.0% of its original size
    Vector2 post = { pos.x - missileTex.width * 0.5f * 0.25f, pos.y - missileTex.height * 0.5f * 0.25f };
    DrawTextureEx(missileTex, post, 0, 0.25f, head);

    EndBlendMode();
}

void Match3GameSession::MyDrawRing(float progress)
{
    Vector2 center = { PADDING + waveOriginC * CELL + CELL * 0.5f + offsetX, PADDING + waveOriginR * CELL + CELL * 0.5f };

    float R = progress * sqrtf((float)(ROWS * ROWS + COLS * COLS)) * CELL;
    float inner = R - 10.0f;
    if (inner < 0)
        inner = 0;
    float outer = R + 30.0f;
    float maxR = sqrtf((float)(ROWS * ROWS + COLS * COLS)) * CELL;
    float fade = 1.0f - clamp01(R / maxR);
    Color ringCol = CHIP_COLORS[waveColor];
    ringCol.a = (unsigned char)(200 * fade);

    BeginBlendMode(BLEND_ADDITIVE);
    DrawRing(center, inner, outer, 0.0f, 360.0f, 64, ringCol);
    EndBlendMode();
}

//End of Match3GameSession.cpp