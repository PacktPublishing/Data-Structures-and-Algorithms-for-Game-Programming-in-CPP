#pragma once

#include "Knight.h"   //Engine includes

#include "../../raylib/examples/models/rlights.h"

class Demo6Light : public Knight
{
public:
	void Start() override;

	FlyThroughCamera* pMainCamera;
	SceneActor* actor = NULL;

	Shader shader = { 0 };

	int ambientLoc = 0;

	Light lights[MAX_LIGHTS] = { 0 };

protected:

	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;
};

