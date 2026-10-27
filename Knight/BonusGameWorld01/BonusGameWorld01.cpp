//BonusGameWorld01 - extra demo to put components together

// This is an extra demo. Its primary purpose is to demonstrate  
// how to combine the features we learned throughout the book.
//
// Unlike other demo projects that focus on a single feature introduced in the book,  
// this project tries to put them all together to build an open game world.
//
// This project is part of a series of bonus demos we will continue to release  
// in the Git repository. We want to show you how we improve and refactor a larger project,  
// transforming it from a ?work-in-progress,? somewhat messy state into a better-structured project.
//
// Think of this demo like this: as you read through most chapters,  
// you might have thought to yourself, ?Why not put everything together??  
// So you take action in one quick step ? copy/paste code, assemble it,  
// and use it as a proof of concept.
//
// The component code used in this sample is introduced in the book:
// - The overall Knight application architecture ? explained in Chapter 2
// - Simulation of daylight changes ? explained in Chapter 4
// - BillboardComponent ? explained in Chapter 7
// - FollowUpCamera ? explained in Chapter 5
// - ParticleComponent ? explained in Chapter 7
// - QuadTreeTerrainComponent ? explained in Chapter 7
// - SkyboxComponent ? explained in Chapter 7
//
// The code is meant to be a demonstration of how to use these components together, 
// rather than a polished game. 
// It's also not optimized for performance, we will discuss performance optimization in the coming sample project.

#include "BonusGameWorld01.h"

int main(int argc, char* argv[])
{
	BonusGameWorld01* KnightBonusGameWorld01 = new BonusGameWorld01();

	KnightBonusGameWorld01->Start();
	KnightBonusGameWorld01->GameLoop();

	delete KnightBonusGameWorld01;
	return 0;
}

BonusGameWorld01::BonusGameWorld01()
{
	_Entities.clear();
}

void BonusGameWorld01::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	//Create a follow-up camera
	pMainCamera = _Scene->CreateSceneObject<FollowUpCamera>("Main Camera");	
	pMainCamera->ShowCursor = true;

	//Initialize entities. See Chapter 2 and 11 for details.
	//Entities are the main game logic objects in this demo, they are derived from Entity class.

	//Initialize a terrain entity. The other entities will use it to get the terrain height.
	_TerrainEntity = new TerrainEntity();
	if (_TerrainEntity->Create(_Scene))
	{
		_Entities.push_back(_TerrainEntity);
	}

	//Initialize a player entity, with finite state machine (FSM) for player control.
	_PlayerEntity = new PlayerEntity();
	if (_PlayerEntity->Create(_Scene, _TerrainEntity))
	{
		_Entities.push_back(_PlayerEntity);
	}

	//Initialize a prop entity, which is a static object with game-specific purpose in 
	// the game world.
	// Here we create a castle prop entity.
	PropEntity* propEntity = new PropEntity();
	if (propEntity->Create(_Scene, _TerrainEntity))
	{
		_Entities.push_back(propEntity);
	}

	//Now make main camera track the movement of the player entity.
	pMainCamera->SetUp(_PlayerEntity->_Actor, 60.0f, 15.0f, CAMERA_PERSPECTIVE);
	//pMainCamera->SetUp(pPlayer->Position, 5.0f, 0, 45, 60, CAMERA_PERSPECTIVE);

	//Create a directional light for rendering shadow.
	sceneLight = _Scene->CreateSceneObject<ShadowSceneLight>("Light");
	sceneLight->SetLight(Vector3{ -50.0f, -30.0f, 50.0f }, WHITE);

	//Create a depth render pass and shadow map render pass.
	pDepthRenderer = new DepthRenderPass(sceneLight);
	pDepthRenderer->Create(_Scene);

	pShadowMapRenderer = new ShadowMapRenderPass(sceneLight, pDepthRenderer->shadowMap.depth.id);
	pShadowMapRenderer->Create(_Scene);

	SetTargetFPS(60); // Set the target frame rate for the game loop
}

void BonusGameWorld01::EndGame()
{
	_Entities.clear();
	__super::EndGame();
}

void BonusGameWorld01::Update(float ElapsedSeconds)
{
	double t = GetTime();

	// Calculate the displacement of the target
	Vector3 oldTarget = pDepthRenderer->pLight->GetCamera3D()->target;

	for(int i=0;i< _Entities.size(); i++)
	{
		_Entities[i]->Update(ElapsedSeconds);
	}
	
	Vector3 cameraPos = _Scene->GetMainCameraActor()->GetPosition();
	SetShaderValue(pShadowMapRenderer->shadowShader, pShadowMapRenderer->shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);

	sceneLight->lightDir = Vector3Normalize(sceneLight->lightDir);
	pDepthRenderer->pLight->GetCamera3D()->position = Vector3Scale(sceneLight->lightDir, -50.0f);
	SetShaderValue(pShadowMapRenderer->shadowShader, pShadowMapRenderer->lightDirLoc, &sceneLight->lightDir, SHADER_UNIFORM_VEC3);

	//These are the controls for moving the light direction.
	//Solely for debugging purpose.
	if (IsKeyDown(KEY_J))
	{
		sceneLight->lightDir.x += cameraSpeed * 60.0f * ElapsedSeconds;
	}

	if (IsKeyDown(KEY_K))
	{
		if (sceneLight->lightDir.x > -0.6f)
			sceneLight->lightDir.x -= cameraSpeed * 60.0f * ElapsedSeconds;
	}

	if (IsKeyDown(KEY_I))
	{
		sceneLight->lightDir.z += cameraSpeed * 60.0f * ElapsedSeconds;
	}

	if (IsKeyDown(KEY_M))
	{
		sceneLight->lightDir.z -= cameraSpeed * 60.0f * ElapsedSeconds;
	}

	if (IsKeyDown(KEY_O))
	{
		sceneLight->lightDir.y += cameraSpeed * 60.0f * ElapsedSeconds;
	}

	if (IsKeyDown(KEY_P))
	{
		sceneLight->lightDir.y -= cameraSpeed * 60.0f * ElapsedSeconds;
	} 

	// Update the scene graph and all SceneObjects.
	__super::Update(ElapsedSeconds);

	//after all the SceneObjects are updated, it's time to make adjustments to the 
	// player position and camera.
	//This is usually done in other game engines during the "Late Update" phase.
	// Knight deoe not have a separate "Late Update" phase, so we do it here.

	// We put player SceneActor at the first level of scene graph, the direct descendent of 
	// the SceneRoot node, its world position is the same as its local position.
	// 
	// In this way we can avoid using GetWorldPosition() method, and it's much easier 
	// to move the player around.
	// 
	// If you put player SceneActor as another SceneActor's child, you would need to update 
	// both its local position and rotation, and then call GetWorldPosition() to get the
	// final world position. Or based on the new world position, you would need to
	// recalculate the local position and rotation of the player SceneActor.
	// 
	// Now, let's adjust player's y position to be above the terrain.
	//

	Vector3 cmpos = pMainCamera->GetPosition();  //local position = world position
	cmpos.y = _PlayerEntity->_Actor->Position.y + 5.0f; //set camera's y position to be above player's y position
	float terrainHeight = _TerrainEntity->_Terrain->GetTerrainY(cmpos.x, cmpos.z);
	if (cmpos.y < (terrainHeight + 5.0f)) {
		cmpos.y = terrainHeight + 5.0f;
		pMainCamera->SetPosition(cmpos); //update camera's local position	
	}

	Vector3 worldPos = _PlayerEntity->_Actor->Position;

	//avoid camera jittery due to sudden change of Actor's y position
	//This hapened when the player is jumping or falling from a steep slope.
	Vector3 LookAt = pMainCamera->GetLookAtPosition();
	LookAt.x = worldPos.x;
	LookAt.z = worldPos.z;

	if (worldPos.y - LookAt.y > 0.1f) {
		LookAt.y += 2.0f * ElapsedSeconds;
	}
	if (worldPos.y - LookAt.y < -0.1f) {
		LookAt.y -= 2.0f * ElapsedSeconds;
	}

	Vector3 targetDisplacement = Vector3Subtract(worldPos, oldTarget);
	// Apply the exact same displacement to the camera's position
	// This is the crucial step to keep the view direction constant!
	pDepthRenderer->pLight->GetCamera3D()->position = Vector3Add(pDepthRenderer->pLight->GetCamera3D()->position, targetDisplacement);

	pMainCamera->SetLookAtPosition(worldPos);
	pMainCamera->Update(0.001f);   //late update to adjust the camera position and look-at direction

	//change of sky light
	sceneLight->lightColor = pMainCamera->GetComponent<SkyboxComponent>()->_SkyColor;

	_FrameUpdateTime = (float)(GetTime() - t);
}

void BonusGameWorld01::DrawOffscreen()
{
	double t = GetTime();

	//Render the scene to the shadow map texture. See Chapter 7 for details.
	pDepthRenderer->BeginShadowMap(_Scene);
	pDepthRenderer->BeginScene();
	pDepthRenderer->Render();
	pDepthRenderer->EndScene();
	pDepthRenderer->EndShadowMap();

	_OffscreenRenderTime = (float)(GetTime() - t);
}

void BonusGameWorld01::DrawFrame()
{
	double t = GetTime();
	//Render the scene with PCF shadow mapping. See Chapter 7 for details.
	pShadowMapRenderer->BeginScene();
	pShadowMapRenderer->Render();
	pShadowMapRenderer->EndScene();
	_FrameRenderTime = (float)(GetTime() - t);
}

void BonusGameWorld01::DrawGUI()
{
	__super::DrawGUI();
	QuadTreeTerrainComponent* pTerrainCmpt = _TerrainEntity->_Terrain;
	//DrawText(TextFormat("Terrain triangle count = %d D:%d S:%d %f %f %f", pTerrainCmpt->NumTriangles, 
		//pDepthRenderer->NumComponentsSkipped, pShadowMapRenderer->NumComponentsSkipped,
		//_FrameUpdateTime * 1000, _OffscreenRenderTime * 1000, _FrameRenderTime * 1000), 10, 160, 50, WHITE);
	//DrawText(TextFormat("LOD Factor: %.1f", pTerrainCmpt->LevelOfDetailDistance), 10, 220, 50, WHITE);
}

void BonusGameWorld01::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();

	UnloadFont(_Font);
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);

	Config.ShowFPS = true; //Enable FPS display.
	Config.EnableDefaultLight = false; //Disable default light. 	
	Config.EnableDefaultRenderPasses = false; //Disable default render passes, we will use ours.
}

