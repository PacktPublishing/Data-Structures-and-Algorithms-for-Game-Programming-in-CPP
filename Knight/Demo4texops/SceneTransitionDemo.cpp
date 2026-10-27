#include "Demo4TexOps.h"

#include <cmath> 

SceneTransitionDemo::SceneTransitionDemo()
{
	title = "Scene Transition Demo";
	description = "This demo loads two graphics files as full screen texture\n\n\nand perform fade in/out transition betwwen two images.";
	currentProgress = 0.0f;
	elapsed_time = 0.0f;
}

void SceneTransitionDemo::Create()
{
	Image art1 = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/mga1.png").c_str());
	scene1 = LoadTextureFromImage(art1);
	UnloadImage(art1);

	Image art2 = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/mga2.png").c_str());
	scene2 = LoadTextureFromImage(art2);
	UnloadImage(art2);

	isReady = true;
}

void SceneTransitionDemo::Update(float elapsed)
{
	elapsed_time += elapsed;
	currentProgress = (std::sin(2 * 3.14159f * 0.1f * elapsed_time) + 1) * 0.5f;
}

void SceneTransitionDemo::Draw2D()
{
	Vector2 position = { (float)(SCREEN_WIDTH - scene1.width) / 2.0f, (float)(SCREEN_HEIGHT - scene1.height) / 2.0f };


	Color c1 = WHITE;
	c1.a = (int)(currentProgress * 255.9f);
	Color c2 = WHITE;
	c2.a = 255 - c1.a;
	DrawTexture(scene1, (int)position.x, (int)position.y, c1);
	DrawTexture(scene2, (int)position.x, (int)position.y, c2);
}

void SceneTransitionDemo::Release()
{
	UnloadTexture(scene1);
	UnloadTexture(scene2);
	isReady = false;
}

