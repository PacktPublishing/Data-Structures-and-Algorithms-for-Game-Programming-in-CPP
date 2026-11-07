#pragma once

#include "Knight.h"   //Engine includes
#include "QuadTreeTerrainComponent.h" // QuadTreeTerrainModelComponent class

class Demo7QuadTreeTerrain : public Knight
{
	public:
		void Start() override;

		FlyThroughCamera* pMainCamera = nullptr;
		SceneActor* pTerrain = nullptr;
		QuadTreeTerrainComponent* pQuadTreeTerrain = nullptr;

	protected:

		void DrawGUI() override;
		void OnCreateDefaultResources() override;
		void OnConfigKnightApp() override;
protected:
	void OnConfigureInput() override;
};


