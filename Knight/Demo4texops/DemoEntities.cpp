#pragma once

#include "Demo4TexOps.h"

Entity::Entity()
{
	isReady = false;
}

SimpleDrawTextureDemo::SimpleDrawTextureDemo()  // Initialize texture to default values
{
   title = "Draw Texture";
   description = "This demo loads a png graphics file as texture and draw it\n\n\n on the center of the screen.";
}

void SimpleDrawTextureDemo::Create()
{
	Image art = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/demoart.png").c_str());
	texture = LoadTextureFromImage(art);
	UnloadImage(art);
	isReady = true;
}

void SimpleDrawTextureDemo::Draw2D()
{
	Vector2 position = { (float)(SCREEN_WIDTH - texture.width) / 2.0f, (float)(SCREEN_HEIGHT - texture.height) / 2.0f };
	DrawTextureV(texture, position, WHITE);
}

void SimpleDrawTextureDemo::Release()
{
	UnloadTexture(texture);
	isReady = false;
}

CompressTextureDemo::CompressTextureDemo()
{
	title = "Comparison of compressed and original texture demo";
	description = "The demo shows two same image but right one is in comprssed format\n\n\nand the left one is in the original uncompressed format.";
}

void CompressTextureDemo::Create()
{
	Image art_org = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/demoart.png").c_str());
	original = LoadTextureFromImage(art_org);
	UnloadImage(art_org);

	Image art_cmp = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/demoart.dds").c_str());
	compressed = LoadTextureFromImage(art_cmp);
	UnloadImage(art_cmp);

	isReady = true;
}

void CompressTextureDemo::Release()
{
	UnloadTexture(original);
	UnloadTexture(compressed);
	isReady = false;
}

void CompressTextureDemo::Draw2D()
{
	Vector2 position = { 0, (float)(SCREEN_HEIGHT - original.height/2) / 2.0f };
	DrawTextureEx(original, position, 0, 0.5f, WHITE);

	position.x += original.width / 2+100;
	DrawTextureEx(compressed, position, 0, 0.5f, WHITE);
}

DrawPartialRotateDemo::DrawPartialRotateDemo()
{
	title = "Draw Partial of Texture and Roatet Demo";
	description = "This demo loads a png graphics file as texture.\n\n\nWe can draw partial image from the soure texture.";
	timeDiff = currentAngle = 0;
}

void DrawPartialRotateDemo::Create()
{
	Image art = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/demoart.png").c_str());
	texture = LoadTextureFromImage(art);
	UnloadImage(art);
	isReady = true;
}

void DrawPartialRotateDemo::Update(float elapsed)
{
	timeDiff = elapsed;
}

void DrawPartialRotateDemo::Draw2D()
{
	Rectangle sr = {393, 6, 698-393, 431-6};
	Vector2 position = { (float)(SCREEN_WIDTH - sr.width) / 2.0f, (float)(SCREEN_HEIGHT - sr.height) / 2.0f };
	Vector2 origin = { 0,0 };

	DrawTextureRec(texture, sr, position, WHITE);

	position = { 300, 300 };
	Rectangle dr = { position.x, position.y, fabs(sr.width), fabs(sr.height) };
	
	currentAngle += 40 * timeDiff;
	if (currentAngle > 360)
		currentAngle -= 360;
	DrawTexturePro(texture, sr, dr, origin, currentAngle, WHITE);
}

void DrawPartialRotateDemo::Release()
{
	UnloadTexture(texture);
	isReady = false;
}
