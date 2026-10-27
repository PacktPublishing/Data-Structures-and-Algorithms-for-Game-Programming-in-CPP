#include "Demo2f.h"

#include <stdlib.h>

#include "../../raylib/examples/core/raygui.h"

void EnemyEntity::Create(Scene* pScene, Entity* pParent)
{
	string name = "Enemy";
	//Place enemy
	Actor = pScene->CreateSceneObject<SceneActor>(name.c_str());
	Actor->Scale = Vector3{ 3.0f, 5.0f, 3.0f };
	Actor->Position = Vector3{ 0, 0.0f, 0.0f };
	ModelComponent* animEnemyComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animEnemyComponent->Load3DModel((Knight::Instance->Config.ResourcesBasePath + "models/gltf/greenman.glb").c_str());
	animEnemyComponent->SetAnimation(0);
}

void EnemyEntity::Update(float elaspedTime)
{
	if (Demo2f::_gameOver == InProgress) {
		//Check if enemy can attack
		if (rechargeTime >= attackCooldown)
		{
			if (_target != NULL && _target->HP > 0)
			{  // countdown end!
				DoAttack();
			}
		}
		else {
			rechargeTime += elaspedTime;
		}
	}

	//process usual entity updates
	__super::Update(elaspedTime);
}

void EnemyEntity::DrawGUI()
{
	float rate = rechargeTime / attackCooldown;
	if (rate > 1)
		rate = 1;

	if (rate < 1)
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xff0000ff);
	else
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0x00ff00ff);
	GuiProgressBar(
		Rectangle{ 1300, 130, 400, 15 },
		Actor->GetName(), (rate < 1)?"Charging":"Ready",
		&rate, 0, 1);

	float health = ((float)HP) / 100.0f;
	if (health < 0.3f)
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xff0000ff);
	else
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xffffffff);
	GuiProgressBar(
		Rectangle{ 1300, 80, 400, 30 },
		"0", "100",
		&health, 0, 1);

}

void EnemyEntity::Die()
{
	__super::Die();

	Demo2f::Log("Enemy dies!");
	Demo2f::GameOver(YouWin);
}



