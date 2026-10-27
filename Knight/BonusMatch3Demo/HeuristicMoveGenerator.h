#pragma once

// This header provides helpers that evaluate swap candidates by scanning local patterns for potential clears and cascades.

#include "Knight.h"

struct Move 
{ 
    int r1, c1, r2, c2; 
};

struct MoveList 
{ 
    std::vector<Move> v; 

    void Clear() 
    { 
        v.clear(); 
    } 
};

static void AddSwap(MoveList& out, int r1, int c1, int r2, int c2) 
{
    if (!in8(r1, c1) || !in8(r2, c2)) 
        return; 
    
    if (abs(r1 - r2) + abs(c1 - c2) != 1) 
        return; 

    out.v.push_back({ r1,c1,r2,c2 });
}

static void GenerateCandidatesFromColor(uint64_t b, MoveList& out) 
{
    //Step 1 - horizontal search
    for (int r = 0; r < 8; ++r) 
    {
        // Extract row r as an 8-bit number in "row" so that bit k corresponds to column k.
        // We AND the bitboard with RowMask(r) to zero out other rows, then shift down to align on bits 0..7.
        uint64_t row = (b & RowMask(r)) >> (r * 8);

        // Pattern H1: X.X   (two same-colored chips with one gap between)
        // Columns k and k+2 must be occupied (bits set). The “hole” is column k+1.
        // If we can swap the hole with a same-colored neighbor from left/right or above/below,
        // that swap can produce a 3-in-a-row horizontally at columns {k, k+1, k+2}.
        for (int k = 0; k <= 5; ++k) 
        { // X.X
            bool x1 = (row >> k) & 1u;    // bit for column k
            bool x2 = (row >> (k + 2)) & 1u;  // bit for column k+2
            
            if (x1 && x2) 
            {
                int hc = k + 1;   // the hole column (k+1)

                // Propose swaps that move something into (r, hc):
                // - From left/right: swap (r,hc) with (r,hc-1) or (r,hc+1).
                // - From vertical neighbors: swap (r,hc) with (r-1,hc) or (r+1,hc).
                // AddSwap will filter any out-of-bounds or non-adjacent pairs.
                AddSwap(out, r, hc, r, hc - 1); 
                AddSwap(out, r, hc, r, hc + 1); 
                AddSwap(out, r, hc, r - 1, hc); 
                AddSwap(out, r, hc, r + 1, hc);
            }
        }

        // Pattern H2: XX.   (two consecutive chips and an empty/right hole at k+2)
        // Columns k and k+1 are set; to complete a 3-in-a-row, we need to bring a chip into column k+2.
        for (int k = 0; k <= 5; ++k) // k+1 <= 6, k+2 <= 7
        { // XX.
            
            bool a = (row >> k) & 1u;    // col k
            bool b2 = (row >> (k + 1)) & 1u;  // col k+1
            
            if (a && b2) 
            {
                int hc = k + 2;   // the hole column is k+2

                AddSwap(out, r, hc, r, hc - 1);   // from left
                AddSwap(out, r, hc, r, hc + 1);   // from right
                AddSwap(out, r, hc, r - 1, hc);   // from above
                AddSwap(out, r, hc, r + 1, hc);   // from below
            }
        }

        // Pattern H3: .XX   (hole on the left, two consecutive chips at k-1 and k)
        // Columns k-1 and k are set; to complete, we need to bring a chip into column k-2.
        for (int k = 2; k < 8; ++k) 
        { // .XX
            bool c = (row >> k) & 1u;    // col k
            bool d = (row >> (k - 1)) & 1u;  // col k-1
            
            if (c && d) 
            {
                int hc = k - 2;   // the hole column is k-2

                // Bring something into (r,hc) via one adjacent swap:
                AddSwap(out, r, hc, r, hc - 1); 
                AddSwap(out, r, hc, r, hc + 1); 
                AddSwap(out, r, hc, r - 1, hc); 
                AddSwap(out, r, hc, r + 1, hc);
            }
        }
    }

	// Step 2 - vertical search
    for (int c = 0; c < 8; ++c) 
    {
        // Build a compact 8-bit column mask "col":
        // bit r (0..7) corresponds to (r, c). This lets us reuse the same logic as rows.
        uint8_t col = 0; 
        
        for (int r = 0; r < 8; ++r) 
            col |= ((b >> (idx8(r, c))) & 1ULL) ? (1u << r) : 0;
       
        // Pattern V1: X.X  (two chips with a gap at row r+1)
        for (int r = 0; r <= 5; ++r) 
        {
            if ((col & (1u << r)) && (col & (1u << (r + 2)))) 
            {
                int hr = r + 1;   // the hole row

                // Move something into (hr,c) from its four neighbors:
                AddSwap(out, hr, c, hr, c - 1);   // left
                AddSwap(out, hr, c, hr, c + 1);   // right
                AddSwap(out, hr, c, hr - 1, c);   // up
                AddSwap(out, hr, c, hr + 1, c);   // down
            }
        }

        // Pattern V2: XX.  (two consecutive chips at r and r+1; hole at r+2)
        for (int r = 0; r <= 5; ++r) 
        {
            if ((col & (1u << r)) && (col & (1u << (r + 1)))) 
            {
                int hr = r + 2;
                AddSwap(out, hr, c, hr, c - 1); 
                AddSwap(out, hr, c, hr, c + 1); 
                AddSwap(out, hr, c, hr - 1, c); 
                AddSwap(out, hr, c, hr + 1, c);
            }
        }

        // Pattern V3: .XX  (hole at r-2; chips at r-1 and r)
        for (int r = 2; r < 8; ++r) 
        {
            if ((col & (1u << r)) && (col & (1u << (r - 1)))) 
            {
                int hr = r - 2;
                AddSwap(out, hr, c, hr, c - 1); 
                AddSwap(out, hr, c, hr, c + 1); 
                AddSwap(out, hr, c, hr - 1, c); 
                AddSwap(out, hr, c, hr + 1, c);
            }
        }
    }
}

//End of HeuristicMoveGenerator.h
