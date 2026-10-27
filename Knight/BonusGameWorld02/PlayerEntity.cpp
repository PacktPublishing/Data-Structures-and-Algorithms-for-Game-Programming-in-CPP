#include "BonusGameWorld02.h"

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

	//Create a sub-SceneActor for attacking effect
	pFireballActor = pScene->CreateSceneObject<SceneActor>("Fireball", _Actor);

	//spacial particle effect for blasting magic
	pAttackEffect = pFireballActor->CreateAndAddComponent<MagicAttackEffect>();
	pAttackEffect->CreateFromFile((Knight::Instance->Config.ResourcesBasePath + "textures/flash00.png").c_str(), 20, Vector3{ 0.0f,0.0f,0.0f }, WHITE, Vector3{ 0,0,0 });

	return true;
}

void PlayerEntity::Update(float elapsedTime)
{
	if (PlayerCharacterFSM)
	{
		PlayerCharacterFSM->Update(elapsedTime);
	}

	Vector3 worldPos = _Actor->GetWorldPosition();
	Scene* pScene = Knight::Instance->_Scene;
	Vector3 camPos = pScene->GetMainCameraActor()->GetPosition();

	Vector3 lpo = Vector3Lerp(worldPos, camPos, 0.3f);
	lpo.y += 0.5f;

	pScene->Lights[1].position = lpo;
	pScene->Lights[1].target = Vector3{ 0,0,0 };
	pScene->Lights[1].color = YELLOW;
	pScene->Lights[1].enabled = true;
	pScene->Lights[1].type = POINT_LIGHT;
	pScene->Lights[1].dirty = true;
	pScene->Lights[1].attn_const = 0.0f;
	pScene->Lights[1].attn_linear = 0.05f;
	pScene->Lights[1].attn_quad = 0.8f;

	if (pAttackEffect && pAttackEffect->isEnabled)
	{
		//update lighting for attacking effect
		pScene->Lights[2].position = pFireballActor->GetWorldPosition();
		pScene->Lights[2].target = Vector3{ 0,0,0 };
		pScene->Lights[2].color = DARKPURPLE;
		pScene->Lights[2].enabled = true;
		pScene->Lights[2].type = POINT_LIGHT;
		pScene->Lights[2].dirty = true;
		pScene->Lights[2].attn_const = 0.0f;
		pScene->Lights[2].attn_linear = 0.02f;
		pScene->Lights[2].attn_quad = 0.03f;
	}
	else 
	{
		pScene->Lights[2].enabled = false;
		pScene->Lights[2].dirty = true;
	}
}

PlayerEntity::~PlayerEntity()
{
	delete PlayerCharacterFSM;
}

//end of PlayerEntity.cpp