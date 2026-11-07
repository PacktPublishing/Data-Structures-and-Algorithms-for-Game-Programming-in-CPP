#pragma once

#include <list>
#include <string>

#include "Knight.h"   //Engine includes
#include "Entities.h"  //Game includes

class Demo2d : public Knight
{
public:
	void Start() override;

protected:
	PerspectiveCamera* camera;

	PlayerEntity* player;
	list<EnemyEntity> enemies;
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


