#pragma once

#include "Knight.h"   //Engine includes

#include <vector>
#include <string>


#include <cmath>
#include <algorithm>
#include <cstdio>
#include <unordered_map>

//Additonal raylib includes used in this sample
#include <rlgl.h>    //We will use some low-level rlgl functions for custom drawing
#include <raymath.h>

// Coverflow UI Layout parameters for you to tweak
const float CARD_SPACING_X = 1.8f;  // The horizontal spacing between cards
const float BASE_Z = 0.0f;   // The base Z position
const float Z_STEP = -0.3f;   // How much further back each side card goes, change this you can do differnet kinds of depth effects
const float SIDE_ANGLE = 45.0f;   // Y-axis rotation for side cards
const float CENTER_SCALE = 1.0f;   // Scale of center card
const float SCALE_DROP = 0.8f;   // Scale reduction per step away from center
const float CARD_Y = 2.4f;    // raised higher
const float CENTER_Z_OFFSET = 1.0f;   // bring center closer
const float SIDE_EXTRA_Z = -0.1f;   // push side cards back

const float REFLECTION_OFFSET_Y = -0.2f; // Slight offset to avoid z-fighting with floor

//Number of actual data items
static const int TOTAL_ITEMS = 1000; 

//Cards visible on each side of center
static const int VISIBLE_RADIUS = 15;   //number of slots visible on each side of the center slot
static const int VISIBLE_SLOTS = VISIBLE_RADIUS * 2 + 1;  //number of visual slots (cards) you can see on screen

//Base card size in world units
static const float CARD_BASE_WIDTH = 2.0f;
static const float CARD_BASE_HEIGHT = 3.0f;

//Max number of textures in cache
static size_t CACHE_CAPACITY = 128;  

//Animation related parameters
const float lerpSpeed = 0.15f;

// A visual card slot (we only ever have VISIBLE_POOL_SIZE of these)
struct Card
{
    Texture2D texture;
    Vector3   position;
    Vector3   targetPosition;

    float rotationY;
    float targetRotationY;

    float scale;
    float targetScale;

    Color tint;
};

// Data for one item in the coverflow
struct ItemData
{
    int         id;
    Color       baseColor;
    std::string texturePath;  // Path to image file for this item
};

// A visual slot in the coverflow (displaying one item as Card)
struct VisualSlot
{
    int  logicalIndex; // Which item this slot currently shows
    int  offsetFromCenter; // -VISIBLE_RADIUS .. +VISIBLE_RADIUS
	bool active; // Is this slot currently used?

    Card card; //current Card in the display slot           
};

// -------------------- LRU Texture Cache --------------------
// TextureCache ONLY handles Texture2D *image file loading*, keyed by string path. 
class TextureCache
{
public:
    explicit TextureCache(size_t capacity)
        : m_capacity(capacity)
    {
    }

    // Get texture for a given file path, loading it on demand.
    // If cache is full, evict least recently used texture.
    Texture2D Get(const std::string& path)
    {
        auto it = m_map.find(path);
        if (it != m_map.end())
        {
            // Move this entry to front (most recently used)
            m_list.splice(m_list.begin(), m_list, it->second);
            return it->second->tex;
        }

        // Not in cache: load a new texture from file
        Texture2D tex = LoadTexture(path.c_str());

        // Evict if needed
        if (m_list.size() >= m_capacity)
        {
            auto lastIt = --m_list.end();  // least recently used (back)
            UnloadTexture(lastIt->tex);    // free GPU resource
            m_map.erase(lastIt->key);
            m_list.erase(lastIt);
        }

        // Insert new entry at front
        CacheEntry entry;
        entry.key = path;
        entry.tex = tex;
        m_list.push_front(entry);
        m_map[path] = m_list.begin();

        return tex;
    }

    // Unload everything (call at shutdown)
    void UnloadAll()
    {
        for (std::list<CacheEntry>::iterator it = m_list.begin();
            it != m_list.end(); ++it)
        {
            UnloadTexture(it->tex);
        }
        m_list.clear();
        m_map.clear();
    }

private:
    struct CacheEntry
    {
        std::string key;
        Texture2D   tex;
    };

    size_t m_capacity;
    std::list<CacheEntry> m_list; // front = most recently used
    std::unordered_map<std::string, std::list<CacheEntry>::iterator> m_map;
};

// BonusCoverFlow3DUI Application Class
class BonusCoverFlow3DUI : public Knight
{
public:
	void Start() override;

    BonusCoverFlow3DUI() : textureCache(CACHE_CAPACITY)
    {
        items.reserve(TOTAL_ITEMS);
        slots.resize(VISIBLE_SLOTS);
    }

protected:

	void OnCreateDefaultResources() override;
	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void DrawGUI() override;

	void EndGame() override;

private:

    PerspectiveCamera* camera = nullptr;

    std::vector<ItemData> items;

    std::vector<VisualSlot> slots;

	int selectedIndex = VISIBLE_RADIUS; // Start with some offset so we can see cards on both sides

    TextureCache textureCache;

    void InitializeSlots();
    void OnSelectionChanged(int delta);
    void ComputeCoverflowTransform(Card& card, int offsetFromCenter);
    void DrawCardWithReflection(const Card& card);

    // Small quick helper to rotate a point around Y axis 
    inline Vector3 RotateY(Vector3 p, float angleDeg)
    {
        float rad = angleDeg * DEG2RAD;
        float c = cosf(rad);
        float s = sinf(rad);
        Vector3 result;
        result.x = p.x * c + p.z * s;
        result.y = p.y;
        result.z = -p.x * s + p.z * c;
        return result;
    }

};

// --- End of BonusCoverflow3DUI.h ---
