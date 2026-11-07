#pragma once

#include <vector>
#include <string>

#include "Knight.h"   //Engine includes
#include "Entities.h"  //Game includes

class Demo2c : public Knight
{
public:
	void Start() override;

protected:
	PerspectiveCamera* camera;

	PlayerEntity* player;
	vector<EnemyEntity> enemies;
	TerrainEntity* terrain;

	void OnCreateDefaultResources() override;
	void Update(float ElapsedSeconds) override;
	void DrawGUI() override;

private:
	void InitEntities();
	void CheckDefeatEnemy();
protected:
	void OnConfigureInput() override;
};

