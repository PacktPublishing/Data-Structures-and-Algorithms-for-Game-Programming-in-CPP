#pragma once

#include "Knight.h"

#include "Match3GameConfig.h"
#include "Bitboards.h"
#include "RandomGenerator.h"
#include "GravitySimulator.h"
#include "HeuristicMoveGenerator.h"

struct Board 
{
    int  cell[ROWS][COLS];     // -1 empty else [0..CHIP_TYPES-1]
    int  special[ROWS][COLS];  // SpecialType (SP_None, SP_CrossBomb, SP_ColorBomb)
    float offX[ROWS][COLS];   // pixel X-offset for anim (e.g., during swap)
    float offY[ROWS][COLS];   // pixel Y-offset for anim (e.g., during fall)
    float highlight[ROWS][COLS];  // transient glow/selection strength [0..1]


    Board() 
    {
        // Initialize empty board with no specials 
        for (int r = 0; r < ROWS; ++r) 
            for (int c = 0; c < COLS; ++c) 
            {
                cell[r][c] = -1; special[r][c] = SP_None; 
                offX[r][c] = offY[r][c] = 0; 
                highlight[r][c] = 0;
            }
    }

    // Quick bounds check (0<=r<ROWS, 0<=c<COLS).
    bool InBounds(int r, int c) const;

    // Fill empty cells with random colors ensuring no immediate 3+ matches.
    void FillNoMatches();

    // Spawn plan for special creation or refills.
    struct SpawnPlan 
    { 
        int r, c; 
        int color; 
        int type; 
    };

    // Detect matches and compute spawn results (e.g., specials) given last swap.
    // Returns number of tiles to clear and outputs spawn list.
    int FindMatchesWithSpawns(std::vector<std::vector<bool>>& mark, const Swap& lastSwap, std::vector<SpawnPlan>& outSpawns) const;

    // Detect matches without spawns (fills 'mark' boolean mask).
    int FindMatches(std::vector<std::vector<bool>>& mark) const;

    // Clear marked cells, apply spawns, and return cleared count.
    int ClearByMask(const std::vector<std::vector<bool>>& mask, const std::vector<SpawnPlan>& spawns);
   
    // True if any grid cell is empty (-1).
    bool AnyEmpty() const;

    // Collapse existing cells vertically (simulate gravity for current holes).
    // Returns true if any movement occurred.
    bool CollapseExisting();

    // Refill newly emptied cells at the top; return true if any new cells spawned.
    bool RefillNew();

    // Score a candidate swap (r1,c1) <-> (r2,c2) using heuristic potential
    // (immediate clears + predicted cascades). Higher is better.
    int ScoreSwapCascadePotential(int r1, int c1, int r2, int c2) const;

    // Try to find any legal move with a decent heuristic score; optionally return score.
    bool FindAnyMoveHeuristic(Vec2i& A, Vec2i& B, int* outScore = nullptr) const;

    // Randomly shuffle board until at least one legal move exists.
    void Shuffle();
};

// End of Match3Board.h
