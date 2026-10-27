#include "Demo4TexOps.h"

#include <cmath>

NPatchDemo::NPatchDemo()
{
	title = "N-Patch Demo";
	description = "This demo draws UI frame with N-patch texture redering.\n\n\nThe border is not affected by the scaling of the image.";
	elapsed_time = 0;
}

void NPatchDemo::Create()
{
	Image art = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/uibkgd.png").c_str());
	_npatchTex = LoadTextureFromImage(art);
	UnloadImage(art);

	Rectangle r = { 0.0f, 0.0f, 2048.0f, 512.0f };
	_ninePatchInfo = { r, 213, 221, 213, 221, NPATCH_NINE_PATCH };

	isReady = true;
}

void NPatchDemo::Update(float elasped)
{
	elapsed_time += elasped;

	float scale = std::sin(3.14159f * 0.2f* elapsed_time) * 0.1f;

	_dest.x = SCREEN_WIDTH * 0.35f;
	_dest.y = SCREEN_HEIGHT * 0.25f;
	_dest.width = _npatchTex.width * (scale+0.8f);
	_dest.height = _npatchTex.height * (scale+1);
}

void NPatchDemo::Draw2D()
{
	Vector2 origin = { _npatchTex.width * 0.25f, _npatchTex.height * 0.25f };
	DrawTextureNPatch(_npatchTex, _ninePatchInfo, _dest, origin, 0.0f, WHITE);
}

void NPatchDemo::Release()
{
	UnloadTexture(_npatchTex);
	isReady = false;
}
