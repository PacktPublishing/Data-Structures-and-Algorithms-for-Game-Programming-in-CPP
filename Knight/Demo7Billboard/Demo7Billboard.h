#pragma once

#include "Knight.h"   //Engine includes
#include "BillboardComponent.h"

class Demo7Billboard : public Knight
{
public:
	void Start() override;

protected:

	void Update(float ElapsedSeconds) override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;

private:
	FlyThroughCamera* mainCamera = nullptr;
	SceneActor* player = nullptr;
	SceneActor* ground = nullptr;
	vector<SceneActor*> imposters = { 0 };
	Texture2D billboardImage = { 0 };
protected:
	void OnConfigureInput() override;
};
