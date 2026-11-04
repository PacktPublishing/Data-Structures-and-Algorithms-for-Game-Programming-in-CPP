#pragma once

#include "Knight.h"   //Engine includes

#include "QuadTreeTerrainComponent.h"
#include "SkyboxComponent.h"
#include "BillboardComponent.h"
#include "ParticleComponent.h"
#include "FollowUpCamera.h"

#include "ShadowSceneLight.h"
#include "ShadowMapRenderPass.h"
#include "LoDShadowMapRenderPass.h"
#include "DepthRenderPass.h"
#include "LoDDepthRenderPass.h"

#include "PlayerFSM.h"

#include "MagicAttackEffect.h"

#include "Entities.h" // Custom entities for the bonus game world demo

class BonusGameWorld02 : public Knight
{
public:
	void Start() override;
	void EndGame() override;

	BonusGameWorld02();

	vector<Entity *> _Entities;

	PlayerEntity* _PlayerEntity = nullptr;

	TerrainEntity* _TerrainEntity = nullptr;

	FollowUpCamera* pMainCamera = nullptr;
	
	//SceneActor* pPlayer = nullptr;
	
	//SceneActor* pTerrain = nullptr;

	Vector3 LookAt = { 0 };

	const float cameraSpeed = 0.05f;



	ShadowSceneLight* sceneLight = nullptr;
	LoDShadowMapRenderPass* pShadowMapRenderer = nullptr;
	LoDDepthRenderPass* pDepthRenderer = nullptr;

protected:

	void OnCreateDefaultResources() override;

	void Update(float ElapsedSeconds) override;
	void DrawOffscreen() override;
	void DrawFrame() override;
	void DrawGUI() override;
	void OnConfigureInput() override;
};

