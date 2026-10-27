#pragma once

#include "Defs.h" 
#include "Scene.h"
#include "SceneObject.h"
#include "SceneActor.h"
#include "SceneCamera.h"
#include "PerspectiveCamera.h"
#include "OrthogonalCamera.h"
#include "FlyThroughCamera.h"
#include "Component.h"
#include "ModelComponent.h"
#include "CubeComponent.h"
#include "SphereComponent.h"
#include "PlaneComponent.h"
#include "CylinderComponent.h"
#include "ConeComponent.h"
#include "SceneRenderPass.h"
#include "LitRenderPass.h"
#include "ForwardRenderPass.h"
#include "LitDepthRenderPass.h"
#include "LitShadowRenderPass.h"
#include "KnightUtils.h"

#include <string>

struct KnightConfig
{
	bool ShowFPS = false;
	bool ShowDebugInfo = false;
	bool EnableDefaultLight = true;
	bool EnableDefaultRenderPasses = true;
	int WindowWidth = DEFAULT_SCREEN_WIDTH;   //use default window width if not set by user
	int WindowHeight = DEFAULT_SCREEN_HEIGHT;    //use default window height if not set by user
	string WindowTitle = DEFAULT_APP_TITLE;
	string ResourcesBasePath = RESOURCE_ROOT_PATH;
};

struct ComparePriorityDescending
{
	bool operator()(SceneRenderPass* lhs, SceneRenderPass* rhs) const
	{
		return lhs->_Priority > rhs->_Priority;
	}
};

class Knight
{
public:
	Knight();
	~Knight();

public:
	virtual void Start();
	void GameLoop();
	virtual void EndGame();
	void ExitGameLoop();

	void DrawText(const char* text, int x, int y, int size, const Color& color = Color{ 255,255,255,255 });
	void SaveScreenshot(const char* fileName);

	KnightConfig Config;

	static Knight* Instance;

	Scene* _Scene;

	float _FrameUpdateTime = 0.0f;
	float _FrameRenderTime = 0.0f;
	float _OffscreenRenderTime = 0.0f;

protected:
	virtual void Update(float ElapsedSeconds);
	virtual void DrawOffscreen();
	virtual void DrawFrame();
	virtual void DrawGUI();

	virtual void ApplyAppConfig();

	void DrawFPS(int x, int y);

	

protected:
	Font _Font;
	bool _shouldExitGameLoop;

	// Render passes registered for offscreen rendering
	multiset<SceneRenderPass*, ComparePriorityDescending> _OffScreenPasses;

	// Render passes registered for frame rendering
	multiset<SceneRenderPass*, ComparePriorityDescending> _RenderPasses;

	// Default resources, such as shaders, textures, etc.
	virtual void OnCreateDefaultResources();
	// Called after the default resources are created, can be used to ensure that all default resources are ready to use.
	virtual void AfterCreateDefaultResources();
	// Called when the default resources are released, can be used to clean up any resources that depend on them.
	virtual void OnReleaseDefaultResources();
	// Called after the default resources are released, can be used to ensure that all resources are cleaned up properly.
	virtual void AfterReleaseDefaultResources();
	// Called to configure the Knight application, such as title, rendering settings.
	virtual void OnConfigKnightApp();
};

