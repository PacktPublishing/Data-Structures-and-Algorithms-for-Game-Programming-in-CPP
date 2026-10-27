#pragma once

#include "Demo2f.h"

Entity::Entity()
{
	Actor = NULL;
}

void Entity::Update(float elaspedTime)
{
}

void TerrainEntity::Create(Scene* pScene, Entity* pParent)
{
	//Create a plan as terrain
	Actor = pScene->CreateSceneObject<SceneActor>("Terrain");
	Actor->Position = Vector3{ 0, -5, 0 };
	Actor->Scale = Vector3{ 20, 1, 20 };
	ModelComponent* animEnemyComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animEnemyComponent->Load3DModel((Knight::Instance->Config.ResourcesBasePath + "models/obj/bridge.obj").c_str());
	Image image = LoadImage((Knight::Instance->Config.ResourcesBasePath + "models/obj/bridge_diffuse.png").c_str());
	Texture2D tex = LoadTextureFromImage(image);
	UnloadImage(image);
	animEnemyComponent->GetModel()->materials[0].maps[0].texture = tex;
}

void Entity::DrawGUI()
{
}

