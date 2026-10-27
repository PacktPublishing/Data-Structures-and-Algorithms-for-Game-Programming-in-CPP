#pragma once

#include "Match3GameConfig.h"

#include <array>

// Helpers for treating the 8x8 board as a 64-bit bitboard.

// idx8: row-major index in [0..63]; 
static inline int idx8(int r, int c) 
{ 
    return r * 8 + c; 
}

//in8: fast bounds check using unsigned wrap.
static inline bool in8(int r, int c) 
{ 
    return (unsigned)r < 8u && (unsigned)c < 8u; 
}

// Precomputed row/column masks for fast horizontal/vertical ops.
static inline uint64_t RowMask(int r) 
{ 
    return 0xFFULL << (r * 8); 
}
static const uint64_t NOT_COL0 = 0xfefefefefefefefeULL;
static const uint64_t NOT_COL7 = 0x7f7f7f7f7f7f7f7fULL;

// One bitboard per color; bit i set means cell i holds that color.
struct Bitboards 
{
    std::array<uint64_t, CHIP_TYPES> bb; // one per color

    // Reset all bitboards.
    void Clear() 
    { 
        bb.fill(0); 
    }

	//Build bitboards from a ROWS x COLS integer grid of colors (-1 means empty).
    void BuildFromGrid(const int grid[ROWS][COLS]);

    // Return a mask with 1s where any color forms a triple (3+ in a line).
    uint64_t AllMatchesMask() const;

    // Expand horizontal triples to cover all 3 cells of each found run.
    // Example: 0b0111 -> 0b0111 (3+ contiguous) per 8-bit row segment.
    static inline uint64_t TriplesHExpanded(uint64_t b) 
    {
        uint64_t out = 0;
        for (int r = 0; r < 8; ++r) {
            uint64_t row = (b & RowMask(r)) >> (r * 8);
            uint64_t a = row & (row >> 1);
            uint64_t s = a & (row >> 2);
            uint64_t ex = s | (s << 1) | (s << 2);
            out |= (ex & 0xFFu) << (r * 8);
        }
        return out;
    }
    
    // Vertical counterpart: mark all bits belonging to any 3-in-a-column.
    static inline uint64_t TriplesVExpanded(uint64_t b) 
    {
        uint64_t a = b & (b >> 8);
        uint64_t s = a & (b >> 16);
        return s | (s << 8) | (s << 16);
    }
    
    // 4-connected flood fill within groupMask, starting from seed bit(s).
    // Useful to compute connected components for special clears.
    static uint64_t FloodFill4(uint64_t groupMask, uint64_t seed) 
    {
        uint64_t comp = 0, frontier = seed & groupMask;
        while (frontier) 
        {
            comp |= frontier;
            uint64_t L = (frontier << 1) & NOT_COL0;
            uint64_t R = (frontier >> 1) & NOT_COL7;
            uint64_t U = (frontier << 8);
            uint64_t D = (frontier >> 8);
            frontier = (L | R | U | D) & groupMask & ~comp;
        }
        return comp;
    }
};

