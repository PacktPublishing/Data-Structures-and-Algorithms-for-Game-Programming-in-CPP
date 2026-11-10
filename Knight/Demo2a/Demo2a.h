#pragma once

#include <string>

#include "Knight.h"   //Engine includes
#include "Entities.h"  //Game includes

class Demo2a : public Knight
{
	public:
		void Start() override;

	protected:
		PerspectiveCamera* camera;

		PlayerEntity* player;
		EnemyEntity* enemy;
		TerrainEntity* terrain;
		PropEntity* prop;

		void Update(float ElapsedSeconds) override;
		void DrawFrame() override;
		void DrawGUI() override;
		void OnConfigureInput() override;

	private:
		void InitEntities();
		void OnCreateDefaultResources() override;
};

