#pragma once

#include <random>

// Multiple RNG streams so effects don't correlate (board vs particles vs hints).
// Each stream is seeded from 'time(nullptr)' with different xorshift constants.
struct RNGStreams 
{
    std::mt19937 board, particles, hint, shuffle;
    
    RNGStreams() 
    {
        unsigned s = (unsigned)time(nullptr);

        // board generation
        board.seed(s ^ 0x9e3779b9u);   
        
        // particle variations
        particles.seed(s ^ 0x243f6a88u);

        // hint search randomness
        hint.seed(s ^ 0xb7e15162u);

        // board shuffling
        shuffle.seed(s ^ 0xdeadbeefu);
    }
    
    // Uniform color selection in [0, types-1] using the 'board' stream.
    int randColorBoard(int types) 
    { 
        std::uniform_int_distribution<int>d(0, types - 1); 
        return d(board); 
    }
    
    // Same, but use the dedicated 'shuffle' stream when shuffling.
    int randColorShuffle(int types) 
    { 
        std::uniform_int_distribution<int> d(0, types - 1); 

        return d(shuffle); 
    }
};


extern RNGStreams gRng;

//End of RandomGenerator.h
