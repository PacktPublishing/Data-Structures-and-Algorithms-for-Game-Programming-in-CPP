#include "Bitboards.h"

// Building bitboards from a match3 grid representation
// Clears all color bitboards, then sets a bit for each occupied cell(per color) so later queries can run in parallel bitwise ops
void Bitboards::BuildFromGrid(const int grid[ROWS][COLS])
{
    Clear();

    for (int r = 0; r < 8; ++r) for (int c = 0; c < 8; ++c) 
    {
        int v = grid[r][c];
        if (v >= 0 && v < 5) 
            bb[(size_t)v] |= (1ULL << idx8(r, c));
    }
}

//For every color’s bitboard, expands any horizontal / vertical triples to full run masks and ORs them together, returning a single mask of all matched cells.
uint64_t Bitboards::AllMatchesMask() const
{
    uint64_t m = 0;
    for (size_t k = 0; k < bb.size(); ++k) 
    {
        uint64_t h = TriplesHExpanded(bb[k]);
        uint64_t v = TriplesVExpanded(bb[k]);
        m |= (h | v);
    }
    return m;
}

//End of Bitboards.cpp