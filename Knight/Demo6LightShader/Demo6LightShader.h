#pragma once

#include "Knight.h"   //Engine includes
#include "SceneLight.h"

class Demo6LightShader : public Knight
{
public:
	void Start() override;

	SceneActor* pPlayer = nullptr;
	SceneLight* pLight = nullptr;

protected:

	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;
	void OnCreateDefaultResources() override;
protected:
	void OnConfigureInput() override;
};

