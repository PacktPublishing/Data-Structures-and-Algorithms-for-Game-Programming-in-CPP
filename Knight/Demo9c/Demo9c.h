#pragma once

#include "A-Star.h"

class Demo9c : public Knight
{
public:
	void Start() override;

protected:
	void Update(float DeltaTime) override;
	void DrawGUI();
	void DrawFrame() override;

	AStarPathFinder _pathFinder;
protected:
	void OnConfigureInput() override;
};

