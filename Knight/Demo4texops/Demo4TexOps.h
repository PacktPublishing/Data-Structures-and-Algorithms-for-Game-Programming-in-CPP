#pragma once

#include <vector>
#include "Knight.h"   //Engine includes
#include "Entities.h"
#include "LRUTextureCacheDemo.h"

#include "../../raylib/examples/core/raygui.h"

class Demo4TexOps : public Knight
{
public:
	void Start() override;

protected:
	PerspectiveCamera* camera;

	void Update(float ElapsedSeconds) override;
	void DrawGUI() override;
	void OnCreateDefaultResources() override;

private:

	vector<Entity*> demos;
	vector<Entity*>::iterator it;

	void InitEntities();
};




