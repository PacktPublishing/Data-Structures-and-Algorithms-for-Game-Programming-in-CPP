#pragma once

#include "raylib.h"
#include "rlgl.h"

#include <cmath>

#include "Knight.h"   //Engine includes

#include "ShadowSceneLight.h"
#include "ShadowMapRenderPass.h"
#include "DepthRenderPass.h"

class Demo7PCFShadow : public Knight
{
public:
	void Start() override;

	FlyThroughCamera* pMainCamera = nullptr;
    SceneActor* Actor = nullptr;
	SceneActor* pFloor = nullptr;
	SceneActor* pProp = nullptr;
	ShadowSceneLight* sceneLight = nullptr;

	ShadowMapRenderPass* pShadowMapRenderer = nullptr;
	DepthRenderPass* pDepthRenderer = nullptr;

	const float cameraSpeed = 0.05f;

	int fc = 0;

protected:

	void Update(float ElapsedSeconds) override;
	void DrawOffscreen() override;
	void DrawFrame() override;
	void DrawGUI() override;

	void OnCreateDefaultResources() override;
	void OnConfigKnightApp() override;
protected:
	void OnConfigureInput() override;
};

