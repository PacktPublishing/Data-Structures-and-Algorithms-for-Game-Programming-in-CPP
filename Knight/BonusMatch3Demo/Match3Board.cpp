#include "Match3Board.h"

extern RNGStreams gRng;

//Fast grid bounds check
bool Board::InBounds(int r, int c) const 
{ 
	return r >= 0 && r < ROWS && c >= 0 && c < COLS; 
}

//Fills the board with random tiles while rejecting any initial 3-in-a-row; verifies with bitboards until no matches remain.
void Board::FillNoMatches()
{
    while (true)
    {
        for (int r = 0; r < ROWS; ++r)
        {
            for (int c = 0; c < COLS; ++c)
            {
                int v;
                do {
                    v = gRng.randColorShuffle(CHIP_TYPES);
					//Do we have any immediate 3-in-a-row with this choice?
                    bool bad = (c >= 2 && v == cell[r][c - 1] && v == cell[r][c - 2]) || (r >= 2 && v == cell[r - 1][c] && v == cell[r - 2][c]);
                    if (!bad)
                        break;
                } while (true);

                cell[r][c] = v; special[r][c] = SP_None;
            }
        }

        Bitboards BB; 
        BB.BuildFromGrid(cell);
        
        if (BB.AllMatchesMask() == 0ULL)
            break;
    }
}

// Marks all matched cells (H+V). 
// Groups connected marked tiles per color, then decides where to spawn specials (cross bomb for 4, color bomb for 5+) preferring 
// the swap origin/target when applicable; returns matched count.
int Board::FindMatchesWithSpawns(std::vector<std::vector<bool>>& mark, const Swap& lastSwap, std::vector<SpawnPlan>& outSpawns) const
{
    mark.assign(ROWS, std::vector<bool>(COLS, false));
    outSpawns.clear();
    int total = 0;
    // H
    for (int r = 0; r < ROWS; ++r)
    {
        int c = 0;
        while (c < COLS)
        {
            if (cell[r][c] < 0)
            {
                ++c;
                continue;
            }
            int k = c + 1;
            while (k < COLS && cell[r][k] == cell[r][c])
                ++k;
            int len = k - c;
            if (len >= 3)
            {
                for (int x = c; x < k; ++x)
                {
                    if (!mark[r][x])
                    {
                        mark[r][x] = true;
                        total++;
                    }
                }
            }
            c = k;
        }
    }

    // V
    for (int c = 0; c < COLS; ++c)
    {
        int r = 0;
        while (r < ROWS)
        {
            if (cell[r][c] < 0)
            {
                ++r;
                continue;
            }
            int k = r + 1;
            while (k < ROWS && cell[k][c] == cell[r][c])
                ++k;
            int len = k - r;
            if (len >= 3)
            {
                for (int y = r; y < k; ++y)
                {
                    if (!mark[y][c])
                    {
                        mark[y][c] = true;
                        total++;
                    }
                }
            }
            r = k;
        }
    }

    if (total == 0)
        return 0;

    // Group marked cells (4-neigh) per color
    struct CC
    {
        std::vector<Vec2i> cells;
        int color;
    };

    std::vector<std::vector<int>> vis(ROWS, std::vector<int>(COLS, 0));

    const int dr[4] = { -1,1,0,0 }, dc[4] = { 0,0,-1,1 };

    std::vector<CC> groups;

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            if (mark[r][c] && !vis[r][c])
            {
                CC g; g.color = cell[r][c];

                std::vector<Vec2i> st;

                st.push_back({ r,c });
                vis[r][c] = 1;

                while (!st.empty())
                {
                    Vec2i v = st.back();
                    st.pop_back();
                    g.cells.push_back(v);
                    for (int i = 0; i < 4; ++i)
                    {
                        int nr = v.r + dr[i], nc = v.c + dc[i];
                        if (InBounds(nr, nc) && mark[nr][nc] && !vis[nr][nc])
                        {
                            vis[nr][nc] = 1;
                            st.push_back({ nr,nc });
                        }
                    }
                }
                groups.push_back(g);
            }
        }
    }

    // pick spawn: prefer swapped cells, else middle
    for (size_t gi = 0; gi < groups.size(); ++gi)
    {
        std::vector<Vec2i>& cells = groups[gi].cells;

        int color = groups[gi].color;
        int sz = (int)cells.size();

        if (sz >= 5 || sz == 4)
        {
            Vec2i p = cells[sz / 2];

            for (size_t i = 0; i < cells.size(); ++i)
            {
                if (cells[i].r == lastSwap.b.r && cells[i].c == lastSwap.b.c)
                {
                    p = cells[i];
                    break;
                }
            }

            for (size_t i = 0; i < cells.size(); ++i)
            {
                if (cells[i].r == lastSwap.a.r && cells[i].c == lastSwap.a.c)
                {
                    p = cells[i];
                    break;
                }
            }

            int tp = (sz >= 5) ? SP_ColorBomb : SP_CrossBomb;
            outSpawns.push_back(SpawnPlan{ p.r,p.c,color,tp });
        }
    }

    return total;
}

//Marks all matched cells (H+V) and returns the number of cells marked; no spawn logic
int Board::FindMatches(std::vector<std::vector<bool>>& mark) const
{
    mark.assign(ROWS, std::vector<bool>(COLS, false));
    int count = 0;
    for (int r = 0; r < ROWS; ++r)
    {
        int run = 1;
        for (int c = 1; c <= COLS; ++c)
        {
            bool same = (c < COLS && cell[r][c] >= 0 && cell[r][c] == cell[r][c - 1]);
            if (same) run++;
            else
            {
                if (run >= 3)
                {
                    for (int k = 0; k < run; ++k)
                    {
                        if (!mark[r][c - 1 - k])
                        {
                            mark[r][c - 1 - k] = true;
                            count++;
                        }
                    }
                }
                run = 1;
            }
        }
    }

    for (int c = 0; c < COLS; ++c)
    {
        int run = 1;
        for (int r = 1; r <= ROWS; ++r)
        {
            bool same = (r < ROWS && cell[r][c] >= 0 && cell[r][c] == cell[r - 1][c]);

            if (same) 
                run++;
            else
            {
                if (run >= 3)
                {
                    for (int k = 0; k < run; ++k)
                    {
                        if (!mark[r - 1 - k][c])
                        {
                            mark[r - 1 - k][c] = true;
                            count++;
                        }
                    }
                }
                run = 1;
            }
        }
    }

    return count;
}

//Clears all marked cells except chosen spawn positions, then applies the spawns and highlights affected cells; returns how many were cleared
int Board::ClearByMask(const std::vector<std::vector<bool>>& mask, const std::vector<SpawnPlan>& spawns)
{
    std::vector<std::vector<bool>> keep(ROWS, std::vector<bool>(COLS, false));

    for (size_t i = 0; i < spawns.size(); ++i)
        keep[spawns[i].r][spawns[i].c] = true;

    int cleared = 0;
    for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c)
    {
        if (mask[r][c] && !keep[r][c])
        {
            cell[r][c] = -1;
            special[r][c] = SP_None;
            highlight[r][c] = 1.0f;
            cleared++;
        }
    }

    for (size_t i = 0; i < spawns.size(); ++i)
    {
        const SpawnPlan& sp = spawns[i];

        cell[sp.r][sp.c] = sp.color;
        special[sp.r][sp.c] = sp.type;
        highlight[sp.r][sp.c] = 1.0f;
    }

    return cleared;
}

//Checks if any cell is empty (-1)
bool Board::AnyEmpty() const
{
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            if (cell[r][c] == -1)
                return true;
    return false;
}

//Plans gravity compaction column-by-column, applies moves (and per-tile fall offsets for animation); returns whether anything moved
bool Board::CollapseExisting()
{
    FallPlan plan;
    plan.Clear();

    for (int c = 0; c < COLS; ++c)
        PlanCompactionForColumn(c, cell, plan.moves);

    if (plan.moves.empty())
        return false;

    ApplyFallMovesAndOffsets(cell, special, offY, plan);
    
    return true;
}

//Spawns new random tiles into empty cells (top-down) and sets initial fall offsets; returns whether any spawned.
bool Board::RefillNew()
{
    bool spawned = false;
    for (int c = 0; c < COLS; ++c)
    {
        for (int r = 0; r < ROWS; ++r)
        {
            if (cell[r][c] == -1)
            {
                cell[r][c] = gRng.randColorBoard(CHIP_TYPES);
                special[r][c] = SP_None;
                offY[r][c] += float((r + 1) * CELL);
                spawned = true;
            }
        }
    }

    return spawned;
}

//Simulates a swap on a copy of the board and repeatedly clears/collapses until stable to estimate the total number of tiles eliminated (cascade potential score).
int Board::ScoreSwapCascadePotential(int r1, int c1, int r2, int c2) const
{
    Board t = *this;

    std::swap(t.cell[r1][c1], t.cell[r2][c2]);
    std::swap(t.special[r1][c1], t.special[r2][c2]);

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            t.offX[r][c] = t.offY[r][c] = 0;
            t.highlight[r][c] = 0;
        }
    }

    int total = 0;
    std::vector<std::vector<bool>> m;

    while (true)
    {
        int found = t.FindMatches(m);
        if (!found)
            break;

        total += found;
        for (int r = 0; r < ROWS; ++r)
        {
            for (int c = 0; c < COLS; ++c)
            {
                if (m[r][c])
                {
                    t.cell[r][c] = -1;
                    t.special[r][c] = SP_None;
                }
            }
        }

        if (!t.CollapseExisting())
            break;
    }
    return total;
}

//Uses bitboard-driven candidate generation + cascade scoring to find a legal swap; breaks ties by favoring greater immediate clears; returns the best move and optional score.
bool Board::FindAnyMoveHeuristic(Vec2i& A, Vec2i& B, int* outScore) const
{
    Bitboards BB;

    BB.BuildFromGrid(cell);

    std::vector<Move> cand;

    for (size_t k = 0; k < BB.bb.size(); ++k)
    {
        MoveList ml;

        GenerateCandidatesFromColor(BB.bb[k], ml);
        cand.insert(cand.end(), ml.v.begin(), ml.v.end());
    }

    // Dedupe
    std::vector<Move> ded;
    for (size_t i = 0; i < cand.size(); ++i)
    {
        bool dup = false;
        for (size_t j = 0; j < ded.size(); ++j)
        {
            const Move& a = cand[i], & b = ded[j];
            if ((a.r1 == b.r1 && a.c1 == b.c1 && a.r2 == b.r2 && a.c2 == b.c2) ||
                (a.r1 == b.r2 && a.c1 == b.c2 && a.r2 == b.r1 && a.c2 == b.c1)) {
                dup = true; 
                break;
            }
        }
        if (!dup)
            ded.push_back(cand[i]);
    }

    if (ded.empty())
    {
        for (int r = 0; r < ROWS; ++r) for (int c = 0; c < COLS; ++c)
        {
            if (c + 1 < COLS) 
                ded.push_back(Move{ r,c,r,c + 1 });

            if (r + 1 < ROWS) 
                ded.push_back(Move{ r,c,r + 1,c });
        }
    }

    int best = -1, bestImm = -1; Vec2i a{ -1,-1 }, b{ -1,-1 };

    std::vector<std::vector<bool>> tmp;
    for (size_t i = 0; i < ded.size(); ++i)
    {
        const Move& m = ded[i];
        int sc = ScoreSwapCascadePotential(m.r1, m.c1, m.r2, m.c2);
        if (sc > 0)
        {
            Board t = *this; std::swap(t.cell[m.r1][m.c1], t.cell[m.r2][m.c2]);
            std::swap(t.special[m.r1][m.c1], t.special[m.r2][m.c2]);

            int im = t.FindMatches(tmp);

            if (sc > best || (sc == best && im > bestImm))
            {
                best = sc;
                bestImm = im;
                a = Vec2i{ m.r1,m.c1 };
                b = Vec2i{ m.r2,m.c2 };
            }
        }
    }

    if (best > 0)
    {
        A = a;
        B = b;
        if (outScore)
            *outScore = best;

        return true;
    }

    return false;
}

//Re-deals all current tiles randomly until the board has no pre-matches and at least one legal move (guarded; falls back to FillNoMatches() if needed). Resets offsets.
void Board::Shuffle()
{
    std::vector<int> pool;
    pool.reserve(ROWS * COLS);

    for (int r = 0; r < ROWS; ++r) 
    {
        for (int c = 0; c < COLS; ++c)
        {
            pool.push_back(cell[r][c]);
            special[r][c] = SP_None;
        }
    }

    std::uniform_int_distribution<int> dist(0, (int)pool.size() - 1);
    std::mt19937& R = gRng.shuffle;

    int guard = 0;
    while (true)
    {
        for (int r = 0; r < ROWS; ++r) 
        {
            for (int c = 0; c < COLS; ++c)
            {
                int idx = dist(R);
                std::swap(pool.back(), pool[idx]);
                cell[r][c] = pool.back();
                pool.pop_back();
            }
        }

        pool.clear(); 
        for (int r = 0; r < ROWS; ++r) 
            for (int c = 0; c < COLS; ++c) 
                pool.push_back(cell[r][c]);

        Bitboards BB; BB.BuildFromGrid(cell);
        if (BB.AllMatchesMask() == 0ULL)
        {
            Vec2i A, B;
            if (FindAnyMoveHeuristic(A, B))
            {
                for (int r = 0; r < ROWS; ++r)
                    for (int c = 0; c < COLS; ++c)
                        offX[r][c] = offY[r][c] = 0;
                break;
            }
        }

        if (++guard > 1000)
        {
            FillNoMatches();
            break;
        }
    }
}

//End of Match3Board.cpp