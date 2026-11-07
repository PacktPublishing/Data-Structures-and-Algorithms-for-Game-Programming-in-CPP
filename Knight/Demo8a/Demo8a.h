#pragma once

#include "Knight.h"

class Demo8a : public Knight
{
public:
	void Start() override;

protected:
	void Update(float ElapsedSeconds) override;
	void DrawGUI();
protected:
	void OnConfigureInput() override;
};

