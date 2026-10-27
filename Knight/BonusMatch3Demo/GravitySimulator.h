#pragma once

// A single falling move: move from (srcR, c) to (dstR, c) within a column.
struct FallPlanEntry 
{ 
    int srcR, dstR, c; 
};

// A plan is a sequence of vertical moves applied to compact a column (or board).
struct FallPlan 
{ 
    std::vector<FallPlanEntry> moves; 

    void Clear() 
    { 
        moves.clear(); 
    } 
};

// Scan one column bottom-up and record moves needed to compact non-empty cells
// toward the bottom; 'write' tracks next destination row.
static void PlanCompactionForColumn(int c, int cell[ROWS][COLS], std::vector<FallPlanEntry>& out) 
{
    int write = ROWS - 1;
    for (int r = ROWS - 1; r >= 0; --r) 
    {
        if (cell[r][c] >= 0) 
        {
            if (write != r) out.push_back({ r, write, c });
            --write;
        }
    }
}

// Apply the planned moves to cell/special arrays and accumulate visual offset
// for falling animation (offY += pixels moved).
static void ApplyFallMovesAndOffsets(int cell[ROWS][COLS], int special[ROWS][COLS], float offY[ROWS][COLS], const FallPlan& plan) 
{
    for (size_t i = 0; i < plan.moves.size(); ++i) 
    {
        FallPlanEntry mv = plan.moves[i];
        int v = cell[mv.srcR][mv.c];
        int sp = special[mv.srcR][mv.c];
        cell[mv.dstR][mv.c] = v;
        special[mv.dstR][mv.c] = sp;
        cell[mv.srcR][mv.c] = -1;
        special[mv.srcR][mv.c] = SP_None;
        offY[mv.dstR][mv.c] += float((mv.srcR - mv.dstR) * CELL);
    }
}

//End of GravitySimulator.h
