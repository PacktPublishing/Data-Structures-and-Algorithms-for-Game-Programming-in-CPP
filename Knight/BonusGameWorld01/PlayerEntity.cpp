#include "BonusGameWorld01.h"

bool PlayerEntity::Create(Scene* pScene, Entity *pTerrain)
{
	//create and place player Actor in the scene
	_Actor = pScene->CreateSceneObject<SceneActor>("Player");
	_Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };

	if (pTerrain)
	{
		pTerrainEntity = dynamic_cast<TerrainEntity*>(pTerrain);
		if (pTerrainEntity == nullptr)
		{
			TraceLog(LOG_INFO, "<PlayerEntity.Create> pTerrain is null, or not a TerrainEntity!");
			return false;
		}
		_Actor->Position = Vector3{ 0.f, pTerrainEntity->_Terrain->GetTerrainY(0,0) ,0.0f };
	}
	
	//load player model and animation
	ModelComponent* animPlayerComponent = _Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->castShadow = Component::eShadowCastingType::Shadow;
	animPlayerComponent->receiveShadow = true;
	animPlayerComponent->Load3DModel((Knight::Instance->Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimationMode(ModelComponent::eAnimMode::Linear_interpolation);
	animPlayerComponent->SetAnimation(6);

	//Initialize player FSM
	PlayerCharacterFSM = new PlayerFSM(_Actor, animPlayerComponent);

	//spacial particle effect for blasting magic
	pAttackEffect = _Actor->CreateAndAddComponent<MagicAttackEffect>();
	pAttackEffect->CreateFromFile((Knight::Instance->Config.ResourcesBasePath + "textures/flash00.png").c_str(), 100, Vector3{ 0.0f,0.5f,0.5f }, WHITE, Vector3{ 0,0,0 });

	return true;
}

void PlayerEntity::Update(float elapsedTime)
{
	if (PlayerCharacterFSM)
	{
		PlayerCharacterFSM->Update(elapsedTime);
	}
	if (pAttackEffect && pAttackEffect->isEnabled)
	{
		pAttackEffect->Update(elapsedTime);
	}

	Vector3 wordPos = _Actor->Position;  //local position = world position 

	float terrainHeight = pTerrainEntity->_Terrain->GetTerrainY(wordPos.x, wordPos.z);
	_Actor->Position.y = terrainHeight + 0.05f; //update player's local/world position
}

PlayerEntity::~PlayerEntity()
{
	delete PlayerCharacterFSM;
}

//end of PlayerEntity.cpp