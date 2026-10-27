#include "Demo4texops.h"

AnimatedTexDemo::AnimatedTexDemo()
{
	title = "Animated Texture Demo";
	description = "This demo draws animated graphics from a spritesheet texture.";
	_currentIdx = 0;
	_anim_time = 0;
	_anim_length = 1.0f;
}

void AnimatedTexDemo::Create()
{
	Image art = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/lightening.png").c_str());
	_spritesSheet = LoadTextureFromImage(art);
	UnloadImage(art);
	isReady = true;
}

void AnimatedTexDemo::Update(float elapsed)
{
	_anim_time += elapsed;
	if (_anim_time >= _anim_length)
		_anim_time -= _anim_length;
}

void AnimatedTexDemo::Draw2D()
{
	int idx = (int)(_anim_time / _anim_length * 11.0f);
	Rectangle src = {0.0f, idx * 128.0f, 1024.0f, 128.0f};
	Vector2 pos = { 300,300 };

	BeginBlendMode(BLEND_ADDITIVE);
	DrawTextureRec(_spritesSheet, src, pos, WHITE);
	EndBlendMode();
}

void AnimatedTexDemo::Release()
{
	UnloadTexture(_spritesSheet);
	isReady = false;
}

