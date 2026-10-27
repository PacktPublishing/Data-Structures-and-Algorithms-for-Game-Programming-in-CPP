#pragma once

#include <vector>

#include "Knight.h"

#include "RandomGenerator.h"
#include "Match3GameConfig.h"

/// <summary>
/// Particle2D - Here we implement a simple 2D particle system for 2D visual effects.
/// </summary>

// Lightweight 2D particle used for chip FX (bursts, explosions, trails).
struct Particle2D 
{
    Vector2 pos, vel;
    float life, lifeMax, size;
    Color col; bool alive;
};

/// <summary>
/// ParticlePool - A pooling system for Particle2D objects.
/// </summary>
struct ParticlePool 
{
    vector<Particle2D> pool;
    vector<int> freeIdx;
    
	//Here we use C++ =11's explicit keyword to prevent implicit conversions for single-argument constructors.
    explicit ParticlePool(size_t cap = 4096) 
    { 
		//reserve enough space for particle pool
        pool.resize(cap); 
        for (int i = (int)cap - 1; i >= 0; --i) 
            freeIdx.push_back(i); 
    }
    
    int Alloc();    
    void Free(int i);
    
    void SpawnBurst(Vector2 center, Color c, int count, float spdMin, float spdMax, float sizeMin, float sizeMax, float lifetime);
    
    void Update(float dt);
    void Draw();


};

//End of ParticlePool.h
