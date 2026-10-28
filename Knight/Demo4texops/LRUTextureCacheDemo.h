#pragma once

#include <unordered_map>
#include <list>
#include <string>

#include "Demo4TexOps.h"

class TextureCache {
private:
    struct CacheEntry {
        Texture2D texture = { 0 };
        list<string>::iterator lru_it;
    };

    unordered_map<string, CacheEntry> cache;
    list<string> lru_list; // Most recently used at front, least at back
    size_t max_size;
    size_t current_size = 0;

public:
    TextureCache(size_t size) : max_size(size) {}

    ~TextureCache() {
        // Unload all textures when the cache is destroyed
        for (auto& pair : cache) {
            UnloadTexture(pair.second.texture);
        }
    }

    Texture2D* GetTexture(const std::string& filePath) {
        // Check if texture is already in cache
        auto it = cache.find(filePath);
        if (it != cache.end()) {
            // Move to front of LRU list (most recently used)
            lru_list.erase(it->second.lru_it);
            lru_list.push_front(filePath);
            it->second.lru_it = lru_list.begin();
            NumHit++;
            return &it->second.texture;
        } else 
            NumMiss++;

        // Texture not in cache, need to load it
        Texture2D texture = LoadTextureForPlatform(filePath.c_str());
        if (texture.id == 0) {
            printf("Failed to load texture:%s\n", filePath.c_str());
            return nullptr;
        }

        // If cache is full, remove least recently used texture
        if (current_size >= max_size && !lru_list.empty()) {
            string lru_key = lru_list.back();
            UnloadTexture(cache[lru_key].texture);
			printf("Unloading texture:%s\n", lru_key.c_str());
            cache.erase(lru_key);
            lru_list.pop_back();
            current_size--;
        }

        // Add new texture to cache
        lru_list.push_front(filePath);
        cache[filePath] = { texture, lru_list.begin() };
        current_size++;

        return &cache[filePath].texture;
    }

    void Clear() {
        for (auto& pair : cache) {
            UnloadTexture(pair.second.texture);
        }
        cache.clear();
        lru_list.clear();
        current_size = 0;
    }

    size_t Size() const { return current_size; }
    size_t MaxSize() const { return max_size; }

	std::string GetTexturePath(int index) const {
		if (index < 0 || index >= lru_list.size()) return "";
		auto it = std::next(lru_list.begin(), index);
		return *it;
	}

    int NumHit = 0;
	int NumMiss = 0;
};


class LRUTextureCacheDemo : public Entity  
{  
public:  

   void Create() override;  
   void Draw2D() override;  
   void Release() override;  

   LRUTextureCacheDemo(int capacity);  

   TextureCache textureCache = { 0 };  
};

//end of LRUTextureCacheDemo.h
