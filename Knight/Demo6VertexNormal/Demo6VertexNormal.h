#pragma once

#include "Knight.h"   //Engine includes

#include "../../raylib/examples/models/rlights.h" //For lighting

class Demo6VertexNormal : public Knight
{
public:
	void Start() override;

	FlyThroughCamera* pMainCamera;
	SceneActor* FNActor = nullptr;
	SceneActor* VNActor = nullptr;

	Shader shader = { 0 };

	int ambientLoc = 0;

	Light lights[MAX_LIGHTS] = { 0 };

protected:

	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;

	Vector3 RotateAroudnY(const Vector3& point, float angle);
};


