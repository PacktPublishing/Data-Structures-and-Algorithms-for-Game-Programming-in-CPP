#pragma once

#include "Knight.h"

class Demo8b : public Knight
{
private:

public:
	void Start() override;

protected:
	void Update(float ElapsedSeconds) override;
	void DrawFrame();
	void DrawGUI();

	void OnCreateDefaultResources();
protected:
	void OnConfigureInput() override;
};

