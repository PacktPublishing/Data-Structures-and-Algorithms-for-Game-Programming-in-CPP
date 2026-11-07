#pragma once

#include "Knight.h"   //Engine includes
#include "TopDownCamera.h"
#include "UnitEntity.h"

#include <vector>

class Demo5RTSCam : public Knight
{
public:
	void Start() override;

	TopDownCamera* RTSCamera = nullptr;
	SceneActor* Actor = nullptr;

	std::vector<UnitEntity> units;

protected:

	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;
protected:
	void OnConfigureInput() override;
};


