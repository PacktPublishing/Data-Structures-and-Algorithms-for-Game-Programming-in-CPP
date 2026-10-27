#pragma once

#include "Knight.h"   //Engine includes
#include "FollowUpCamera.h"
#include "TopDownCamera.h"

class Demo5MultiCams : public Knight
{
	public:
		void Start() override;

		SceneActor* Actor = NULL;

		FollowUpCamera* pChaseCamera = nullptr;
		TopDownCamera* pTopDownCamera = nullptr;

		Rectangle splitScreenRect = { 0 };
		RenderTexture ChaseCamRT = { 0 };
		RenderTexture TopDownCamRT = { 0 };

	protected:

		void Update(float ElapsedSeconds) override;
		void DrawOffscreen() override;
		void DrawFrame() override;
		void DrawGUI() override;
		void OnCreateDefaultResources() override;

		void DrawGameWorld(SceneCamera* pCam);

		virtual void OnConfigKnightApp();
};


