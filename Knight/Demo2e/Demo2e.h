#pragma once

#include <list>
#include <string>

#include "Knight.h"   //Engine includes

#include "Demo2eUI.h"

class Demo2e : public Knight
{
public:
	void Start() override;

protected:
	Demo2eUIManager Manager;

	void Update(float ElapsedSeconds) override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;
protected:
	void OnConfigureInput() override;
};


