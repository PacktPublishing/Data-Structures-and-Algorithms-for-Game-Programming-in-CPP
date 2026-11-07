#pragma once

#include "raylib.h"

#include "Knight.h"   //Engine includes

#include "ParticleComponent.h"

class Demo7Particle : public Knight
{
public:
	void Start() override;

	FlyThroughCamera* pMainCamera;
	SceneActor* Actor = NULL;

	Vector3 fountainOrigin = { 0.0f, 0.0f, 0.0f };

	

	ParticleComponent* pParticle;

	float time = 0.0f;

	Shader shader = { 0 };
	Mesh mesh = { 0 };

	// Uniform locations
	int timeLoc = -1;
	int lifetimeLoc = -1;
	int scaleLoc = -1;
	int cameraRightLoc = -1;
	int cameraUpLoc = -1;
	int mvpLoc = -1;

	Vector3 cameraRight = { 0 };
	Vector3 cameraUp = { 0 };

	vector<Particle> particles;

	

protected:

	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void OnCreateDefaultResources() override;
protected:
	void OnConfigureInput() override;
};

