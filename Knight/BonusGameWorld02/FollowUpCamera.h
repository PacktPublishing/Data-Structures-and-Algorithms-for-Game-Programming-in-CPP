//Same FollowUpCamera as in the Demo53PV project
#pragma once

#include "SceneObject.h"
#include "raylib.h"
#include "raymath.h" 
#include "SceneCamera.h"

class FollowUpCamera : public SceneCamera
{
public:

	FollowUpCamera(Scene* Scene, const char* Name = nullptr, bool IsMainCamera = true);

	void SetUp(SceneActor* pTarget, float fovY, float defaultDistance, int projType);

	bool Update(float ElapsedSeconds) override;

	//target Actor to follow up
	SceneActor *TargetActor = NULL;

	// Variables to control the camera's distance from the player
	float cameraDistance = 5.0f;

	bool processMouseInput = true;

private:

	float mCameraHorizontalAngleShift = 0.0f;
	float mCameraVerticalOffset = 0.0f;
};
