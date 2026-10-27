#pragma once

#include "Knight.h"
#include "LRUTextureCacheDemo.h"

#include <vector>

vector<string> texturePaths = {
    "textures/PokerDeckCards/12.png",
    "textures/PokerDeckCards/25.png",
    "textures/PokerDeckCards/50.png",
    "textures/PokerDeckCards/52.png"
};

LRUTextureCacheDemo::LRUTextureCacheDemo(int capacity) : textureCache(capacity)
{
	title = "LRU Texture Cache Demo";
	description = "Implement a simple LRU texture cache to avoid loading duplicate textures.";
}

void LRUTextureCacheDemo::Create()
{
	isReady = true;
}

void LRUTextureCacheDemo::Draw2D()
{
    int index = ((int)GetTime()/2) % texturePaths.size();

    Texture2D* texture = textureCache.GetTexture(Knight::Instance->Config.ResourcesBasePath + texturePaths[index]);

    // Draw the texture
    DrawTexture(*texture, SCREEN_WIDTH / 2 - texture->width / 2, SCREEN_HEIGHT / 2 - texture->height / 2, WHITE);

    // Draw cache info
    Knight::Instance->DrawText(TextFormat("Cache Size: %d/%d Hit:%d, Miss:%d", 
        textureCache.Size(), textureCache.MaxSize(), textureCache.NumHit, textureCache.NumMiss), 10, 30, 20, WHITE);
	for (int i = 0; i < textureCache.Size(); i++) {
        Knight::Instance->DrawText(TextFormat("Texture:%s", textureCache.GetTexturePath(i).c_str()), 15, 75 + i * 35, 30, (i == index) ? GREEN : WHITE);
	}
}

void LRUTextureCacheDemo::Release() 
{

	isReady = false;
}
