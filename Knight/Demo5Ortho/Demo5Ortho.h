#pragma once

#include "Knight.h"   //Engine includes

class Demo5Ortho : public Knight
{
public:
	void Start() override;

	Demo5Ortho();

	OrthogonalCamera* OrthCam = NULL;

protected:

	void DrawFrame() override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;
protected:
	void OnConfigureInput() override;
};

