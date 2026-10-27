#pragma once

#include "Knight.h"

#include <unordered_map>
#include <vector>
#include <random>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <raymath.h>


// Entities (cubes) 
struct CubeEnt {
    Vector3 pos;     // xz move, sits on ground
    Vector3 vel;     // xz velocity
    float   half;    // half-size (uniform)
    Color   base;
    bool    highlight; // area query
    //bool    hit;       // ray picked

    CubeEnt() : pos(), vel(), half(0.5f), base{ 195,215,235,255 }, highlight(false) {}
    // AABB in world space (y from 0 to 2*half so it sits on ground)
    inline Vector3 min() const { return Vector3 { pos.x - half, 0.0f, pos.z - half }; }
    inline Vector3 max() const { return Vector3 { pos.x + half, 2.0f * half, pos.z + half }; }
};

// Hash Cell (XZ) 
struct Cell { int x, z; };
static inline bool operator==(const Cell& a, const Cell& b) { return a.x == b.x && a.z == b.z; }

struct CellHash {
    size_t operator()(const Cell& c) const {
        uint64_t k = (uint64_t)(uint32_t)c.x << 32 | (uint32_t)c.z;
        k ^= (k >> 33); k *= 0xff51afd7ed558ccdULL;
        k ^= (k >> 33); k *= 0xc4ceb9fe1a85ec53ULL;
        k ^= (k >> 33);
        return (size_t)k;
    }
};

static inline Cell CellForXZ(const Vector3& p, float cell) {
    return Cell{ (int)std::floor(p.x / cell), (int)std::floor(p.z / cell) };
}

// DDA on XZ plane 
struct DDA2D {
    int ix, iz, stepX, stepZ;
    float t, tMaxX, tMaxZ, tDeltaX, tDeltaZ;
};

static inline DDA2D InitDDA_XZ(const Ray& ray, float tStart, float cellSize)
{
    DDA2D d;
    d.t = tStart;

    Vector3 p = Vector3Add(ray.position, Vector3Scale(ray.direction, tStart));
    d.ix = (int)std::floor(p.x / cellSize);
    d.iz = (int)std::floor(p.z / cellSize);
    d.stepX = (ray.direction.x > 0.0f) ? 1 : -1;
    d.stepZ = (ray.direction.z > 0.0f) ? 1 : -1;

    if (std::fabs(ray.direction.x) < 1e-8f) {
        d.tMaxX = d.tDeltaX = 1e30f;
    }
    else {
        float edgeX = (d.stepX > 0) ? ((d.ix + 1) * cellSize) : (d.ix * cellSize);
        d.tMaxX = tStart + (edgeX - p.x) / ray.direction.x;
        d.tDeltaX = cellSize / std::fabs(ray.direction.x);
    }
    if (std::fabs(ray.direction.z) < 1e-8f) {
        d.tMaxZ = d.tDeltaZ = 1e30f;
    }
    else {
        float edgeZ = (d.stepZ > 0) ? ((d.iz + 1) * cellSize) : (d.iz * cellSize);
        d.tMaxZ = tStart + (edgeZ - p.z) / ray.direction.z;
        d.tDeltaZ = cellSize / std::fabs(ray.direction.z);
    }
    return d;
}

static inline void DDA_Step_XZ(DDA2D& d)
{
    if (d.tMaxX < d.tMaxZ) { d.ix += d.stepX; d.t = d.tMaxX; d.tMaxX += d.tDeltaX; }
    else { d.iz += d.stepZ; d.t = d.tMaxZ; d.tMaxZ += d.tDeltaZ; }
}

// Geometry helpers 
static inline bool RayAABB(const Ray& ray, const Vector3& bmin, const Vector3& bmax, float& tHit)
{
    // Slabs method
    float tmin = -1e30f, tmax = 1e30f;

    // X
    if (std::fabs(ray.direction.x) < 1e-8f) {
        if (ray.position.x < bmin.x || ray.position.x > bmax.x) return false;
    }
    else {
        float inv = 1.0f / ray.direction.x;
        float t1 = (bmin.x - ray.position.x) * inv;
        float t2 = (bmax.x - ray.position.x) * inv;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1); tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }
    // Y
    if (std::fabs(ray.direction.y) < 1e-8f) {
        if (ray.position.y < bmin.y || ray.position.y > bmax.y) return false;
    }
    else {
        float inv = 1.0f / ray.direction.y;
        float t1 = (bmin.y - ray.position.y) * inv;
        float t2 = (bmax.y - ray.position.y) * inv;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1); tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }
    // Z
    if (std::fabs(ray.direction.z) < 1e-8f) {
        if (ray.position.z < bmin.z || ray.position.z > bmax.z) return false;
    }
    else {
        float inv = 1.0f / ray.direction.z;
        float t1 = (bmin.z - ray.position.z) * inv;
        float t2 = (bmax.z - ray.position.z) * inv;
        if (t1 > t2) std::swap(t1, t2);
        tmin = std::max(tmin, t1); tmax = std::min(tmax, t2);
        if (tmin > tmax) return false;
    }
    if (tmax <= 0.0f) return false; // box is behind
    tHit = (tmin > 0.0f) ? tmin : tmax;
    return tHit > 0.0f;
}

static inline bool CircleIntersectsAABB_XZ(const Vector3& center, float radius,
    const Vector3& bmin, const Vector3& bmax)
{
    // distance from circle center to AABB on XZ
    float dx = std::max(0.0f, std::max(bmin.x - center.x, center.x - bmax.x));
    float dz = std::max(0.0f, std::max(bmin.z - center.z, center.z - bmax.z));
    return (dx * dx + dz * dz) <= radius * radius;
}


class BonusHashGridDemo : public Knight
{
public:
	void Start() override;

protected:
	void OnCreateDefaultResources() override;
	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void DrawGUI() override;

private:
	// World
	const float CELL = 2.0f;     // hash grid cell
	const float HALF = 60.0f;    // half-extent
	const int   NUM = 1500;     // cubes
	const float DT = 1.0f / 60.0f;

	// Area query
	float   areaRadius = 6.0f;
	Vector3 areaCenter = Vector3{ 0,0,0 };
	bool    areaActive = false;

	std::vector<CubeEnt> ents;
    int selectIdx = -1;

	// Sparse hash: Cell -> indices
	std::unordered_map<Cell, std::vector<int>, CellHash> grid;
	std::vector<Matrix> transforms;
	FlyThroughCamera* camera = nullptr;

    inline void EnsureRadiusRange()
    {
        if (areaRadius < 2.0f)  areaRadius = 2.0f;
        if (areaRadius > 40.0f) areaRadius = 40.0f;
	}
};

