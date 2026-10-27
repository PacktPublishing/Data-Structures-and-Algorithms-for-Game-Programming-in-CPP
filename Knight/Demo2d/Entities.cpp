#include "Demo2d.h"

//Constructor of base class Entity
Entity::Entity()
{
	Actor = NULL;
	HP = 100;
}

//Update method of base class Entity
void Entity::Update(float elaspedTime)
{
}

//Create player entity with a SceneActor and animations
void PlayerEntity::Create(Scene* pScene, Entity* pParent)
{
	//Place player
	Actor = pScene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 3.0f, 3.0f, 3.0f };
	Actor->Position = Vector3{ 0, -3, 30.0f };
	Actor->Rotation.y = 180.0f;
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Knight::Instance->Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(6);
}

//Constructor of EnemyEntity
EnemyEntity::EnemyEntity()
{
	respawnInterval = -1.0f;  // nagative value means not in respawn countdown
}

//Create enemy entity with a SceneActor
void EnemyEntity::Create(Scene* pScene, Entity* pParent)
{
	int x = ((rand() % 4) - 2) * 10;
	string name = "Enemy" + std::to_string(x);
	//Place enemy
	Actor = pScene->CreateSceneObject<SceneActor>(name.c_str());
	Actor->Scale = Vector3{ 3.0f, 5.0f, 3.0f };
	Actor->Position = Vector3{ (float)x, 0.0f, 0.0f };
	ModelComponent* animEnemyComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animEnemyComponent->Load3DModel((Knight::Instance->Config.ResourcesBasePath + "models/gltf/greenman.glb").c_str());
	animEnemyComponent->SetAnimation(0);
}

//Update method of EnemyEntity, handle respawn logic
void EnemyEntity::Update(float elaspedTime)
{
	if (respawnInterval > 0.0f) //Are we in respawn countdown?
	{
		respawnInterval -= elaspedTime;
		if (respawnInterval <= 0.0f)
		{  // countdown end!
			Resurrect();
		}
	}
}

//Die() function of EnemyEntity, set HP to 0 and deactivate the actor
void EnemyEntity::Die()
{
	HP = 0;
	Actor->IsActive = false;
	respawnInterval = 5.0f;
}

//Resurrect() function of EnemyEntity, set HP to 100 and activate the actor
void EnemyEntity::Resurrect()
{
	HP = 100;
	Actor->IsActive = true;
	respawnInterval = -1.0f;
}

//Create terrain entity with a SceneActor and a model
void TerrainEntity::Create(Scene* pScene, Entity* pParent)
{
	//Create a plan as terrain
	Actor = pScene->CreateSceneObject<SceneActor>("Terrain");
	Actor->Position = Vector3{ 0, -5, 0 };
	Actor->Scale = Vector3{ 20, 1, 20 };
	ModelComponent* animEnemyComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animEnemyComponent->Load3DModel((Knight::Instance->Config.ResourcesBasePath + "models/obj/bridge.obj").c_str(), (Knight::Instance->Config.ResourcesBasePath + "models/obj/bridge_diffuse.png").c_str());
}

//End of Entities.cpp
