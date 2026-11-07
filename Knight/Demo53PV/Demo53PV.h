#pragma once

#include "Knight.h"   //Engine includes
#include "FollowUpCamera.h"

#define MAX_COLUMNS 20

class Demo53PV : public Knight
{
	public:
		void Start() override;
	
		FollowUpCamera* ChaseCamera = nullptr;
		SceneActor* Actor = nullptr;

	protected:

		void Update(float ElapsedSeconds) override;
		void DrawFrame() override;
		void DrawGUI() override;
		void OnCreateDefaultResources();
protected:
	void OnConfigureInput() override;
};


