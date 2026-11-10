#pragma once

#include "raylib.h"
#include "rlgl.h"

#include "SceneActor.h"
#include "Component.h"

class SkyboxComponent : public Component
{
public:

	SkyboxComponent();
	~SkyboxComponent();

	bool CreateFromFile(const char* pSkyboxTexFilePath, CubemapLayout layout, float skyboxScale, bool useHdr);

	void Draw(RenderHints *pRH = nullptr) override;

	Shader shader = { 0 };
	Shader shdrCubemap = { 0 };

	// Load skybox model
	Mesh cube = { 0 };
	Model skybox = { 0 };
	bool useHDR = false;
	float scale = 5.0f;

	friend SceneActor;

};
