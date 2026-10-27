#pragma once

#include "Demo2f.h"

#include "../../raylib/examples/core/raygui.h"

#include <stdlib.h>

void PlayerEntity::Create(Scene* pScene, Entity* pParent)
{
	//Place player
	Actor = pScene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 3.0f, 3.0f, 3.0f };
	Actor->Position = Vector3{ 0, -3, 30.0f };
	Actor->Rotation.y = 180.0f;
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel("../../resources/models/gltf/robot.glb");
	animPlayerComponent->SetAnimation(6);
}

void PlayerEntity::Update(float elaspedTime)
{
	if (rechargeTime >= attackCooldown) 
	{
		if (IsKeyPressed(KEY_SPACE) && _target != NULL && _target->HP > 0)
			DoAttack();
	}
	else {
		rechargeTime += elaspedTime;
	}
	__super::Update(elaspedTime);
}

void PlayerEntity::DrawGUI()
{
	float rate = rechargeTime / attackCooldown;
	if (rate > 1)
		rate = 1;

	if (rate < 1)
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xff0000ff);
	else
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0x00ff00ff);
	GuiProgressBar(
		Rectangle{ 100, 130, 400, 15 },
		Actor->GetName(), (rate < 1) ? "Charging" : "Ready",
		&rate, 0, 1);

	float health = ((float)HP) / 100.0f;
	if (health < 0.3f)
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xff0000ff);
	else 
		GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xffffffff);
	GuiProgressBar(
		Rectangle{ 100, 80, 400, 30 },
		"0", "100",
		&health, 0, 1);
}

void PlayerEntity::Die()
{
	__super::Die();

	Demo2f::Log("Player dies!");
	Demo2f::GameOver(YouLose);
}


