#include "RandomGenerator.h"
#include "ParticlePool.h"

int ParticlePool::Alloc()
{
    if (freeIdx.empty())
        return -1;
    int i = freeIdx.back();
    freeIdx.pop_back();
    return i;
}

void ParticlePool::Free(int i)
{
    pool[i].alive = false;
    freeIdx.push_back(i);
}

void ParticlePool::SpawnBurst(Vector2 center, Color c, int count, float spdMin, float spdMax, float sizeMin, float sizeMax, float lifetime)
{
    mt19937& R = gRng.particles;
    uniform_real_distribution<float> ang(0, 2 * PI);
    uniform_real_distribution<float> spd(spdMin, spdMax);
    uniform_real_distribution<float> sz(sizeMin, sizeMax);

    for (int k = 0; k < count; k++)
    {
        int i = Alloc();
        if (i < 0)
            break;

        float a = ang(R), s = spd(R);

        Particle2D p;

        p.pos = center;
        p.vel = { cosf(a) * s, sinf(a) * s - s * 0.25f };
        p.life = p.lifeMax = lifetime;
        p.size = sz(R);
        p.col = c;
        p.col.a = 220;
        p.alive = true;
        pool[i] = p;
    }
}

void ParticlePool::Update(float dt)
{
    for (size_t i = 0; i < pool.size(); ++i)
    {
        Particle2D& p = pool[i];

        if (!p.alive)
            continue;

        p.vel.x *= PARTICLE_DRAG;
        p.vel.y = p.vel.y * PARTICLE_DRAG + PARTICLE_GRAV_Y * dt;
        p.pos.x += p.vel.x * dt;
        p.pos.y += p.vel.y * dt;
        p.life -= dt; if (p.life <= 0) Free((int)i);
    }
}

void ParticlePool::Draw()
{
    BeginBlendMode(BLEND_ADDITIVE);
    for (size_t i = 0; i < pool.size(); ++i)
    {
        Particle2D& p = pool[i];

        if (!p.alive)
            continue;

        float a = clamp01(p.life / p.lifeMax);
        Color col = p.col;
        col.a = (unsigned char)(255 * a);
        DrawCircleV(p.pos, p.size, col);
    }

    EndBlendMode();
}

//End of ParticlePool.cpp