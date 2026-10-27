#include "Demo4TexOps.h"

GlowDemo::GlowDemo()
{
	title = "Glow Effect Demo";
	description = "This demo use additive blending mode to draw glowing effect.";
	currentAngle = 0;
}

void GlowDemo::Create()
{
	Image art1 = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/mga1.png").c_str());
	background = LoadTextureFromImage(art1);
	UnloadImage(art1);

	Image art2 = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/glow.png").c_str());
	glow = LoadTextureFromImage(art2);
	UnloadImage(art2);

	isReady = true;
}

void GlowDemo::Update(float elasped)
{
	currentAngle += elasped * 30;
	if (currentAngle > 360.0f)
		currentAngle -= 360.0f;
}


void GlowDemo::Draw2D()
{
	float scale = 2.0f;
	Vector2 position = { (float)(SCREEN_WIDTH - background.width) / 2.0f, (float)(SCREEN_HEIGHT - background.height) / 2.0f };
	Rectangle source = { 0.0f, 0.0f, (float)glow.width, (float)glow.height };
	Rectangle dest = { position.x+778, position.y+536, (float)glow.width * scale, (float)glow.height * scale };
	Vector2 origin = { (float)glow.width * scale /2, (float)glow.height * scale /2 };

	DrawTexture(background, (int)position.x, (int)position.y, WHITE);
	BeginBlendMode(BLEND_ADDITIVE);
	DrawTexturePro(glow, source, dest, origin,currentAngle, YELLOW);
	EndBlendMode();
}

void GlowDemo::Release()
{
	UnloadTexture(background);
	UnloadTexture(glow);
	isReady = false;
}
