#pragma once

#include <string>

#include "Knight.h"   //Engine includes
#include "Entities.h"  //Game includes

#define MAX_NUM_ENEMIES 3

class Demo2b : public Knight
{
public:
	void Start() override;

protected:
	PerspectiveCamera* camera;

	PlayerEntity* player;
	EnemyEntity enemies[MAX_NUM_ENEMIES];
	TerrainEntity* terrain;

	void Update(float ElapsedSeconds) override;
	void DrawGUI() override;

private:
	void InitEntities();
	bool CheckDefeatEnemy();
	void OnCreateDefaultResources() override;
protected:
	void OnConfigureInput() override;
};

