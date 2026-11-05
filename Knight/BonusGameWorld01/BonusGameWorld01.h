//Bonus Game World Demo - Main header file.
//Read the description in BonusGameWorld01.cpp for details.
#pragma once

#include "Knight.h"   
#include "InputConfig.h"

#include "QuadTreeTerrainComponent.h"
#include "SkyboxComponent.h"
#include "BillboardComponent.h"
#include "ParticleComponent.h"
#include "FollowUpCamera.h"

#include "ShadowSceneLight.h"
#include "ShadowMapRenderPass.h"
#include "DepthRenderPass.h"

#include "PlayerFSM.h"

#include "MagicAttackEffect.h"

#include "Entities.h" // Custom entities for the bonus game world demo

class BonusGameWorld01 : public Knight
{
public:
	void Start() override;
	void EndGame() override;

	BonusGameWorld01();

	vector<Entity *> _Entities;

	PlayerEntity* _PlayerEntity = nullptr;

	TerrainEntity* _TerrainEntity = nullptr;

	FollowUpCamera* pMainCamera = nullptr;
	
	//SceneActor* pPlayer = nullptr;
	
	//SceneActor* pTerrain = nullptr;

	Vector3 LookAt = { 0 };



	const float cameraSpeed = 0.05f;



	ShadowSceneLight* sceneLight = nullptr;
	ShadowMapRenderPass* pShadowMapRenderer = nullptr;
	DepthRenderPass* pDepthRenderer = nullptr;

protected:

	void OnCreateDefaultResources() override;

	void Update(float ElapsedSeconds) override;
	void DrawOffscreen() override;
	void DrawFrame() override;
	void DrawGUI() override;
	void OnConfigureInput() override;
};

