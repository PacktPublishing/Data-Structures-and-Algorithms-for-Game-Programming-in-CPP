#pragma once

#include "Knight.h"   //Engine includes

class Demo6NormalMap : public Knight
{
public:
	void Start() override;
	void EndGame() override;

	FlyThroughCamera* pMainCamera = nullptr;
	SceneActor* Actor = nullptr;

	Vector3 modelpos = { 0,0,0 };
	Vector3 modelpos2 = { -2,0,0 };
	Texture2D diffuse = { 0 };
	Texture2D normalMap = { 0 };

	Shader shader = { 0 };

	Vector3 lightPos = { -0.33f, 0.0f, 4.14f };
	int lightPosLoc = -1;
	int lightPowerLoc = -1;
	int lightColorLoc = -1;
	Vector3 lightColor = { 1,1,1 };
	float lightPower = 20.0f;

protected:

	SceneActor* characterActor = nullptr;
	SceneActor* cubeActor = nullptr;

	void OnCreateDefaultResources() override;
	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void DrawGUI() override;
protected:
	void OnConfigureInput() override;
};


