#include "Demo7PCFShadow.h"

int main(int argc, char* argv[])
{
	Demo7PCFShadow* KnightDemo7PCFShadow = new Demo7PCFShadow();
	KnightDemo7PCFShadow->Start();
	KnightDemo7PCFShadow->GameLoop();
	delete KnightDemo7PCFShadow;
	return 0;
}

void Demo7PCFShadow::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();
	Config.ShowFPS = true;
	
	pMainCamera = _Scene->CreateSceneObject<FlyThroughCamera>("Main Camera");
	pMainCamera->SetUp(Vector3{ 0,0,0 }, 60, 0, 60, 45, CAMERA_PERSPECTIVE);
	
	sceneLight = _Scene->CreateSceneObject<ShadowSceneLight>("Light");
	sceneLight->SetLight(Vector3{ -25, -60, -30 }, WHITE);
	
	pFloor = _Scene->CreateSceneObject<SceneActor>("Terrain");
	pFloor->Position = Vector3{ 0, -3, 0 };
	pFloor->Scale = Vector3{ 2, 1, 2 };	

	pFloor = _Scene->CreateSceneObject<SceneActor>("Floor");
	Mesh floorMesh = GenMeshCube(32.0f, 0.1f, 32.0f);
	ModelComponent* floorComponent = pFloor->CreateAndAddComponent<ModelComponent>();
	floorComponent->LoadFromMesh(floorMesh, (Config.ResourcesBasePath + "textures/p8.png").c_str());
	
	pProp = _Scene->CreateSceneObject<SceneActor>("Prop");
	pProp->Scale = Vector3{ 0.1f, 0.1f, 0.1f };
	pProp->Position = Vector3{ 2.f,0.7f,6.0f };
	pProp->Rotation = Vector3{ 0,0,0 };
	
	ModelComponent* propModelComponent = pProp->CreateAndAddComponent<ModelComponent>();
	propModelComponent->Load3DModel((Config.ResourcesBasePath + "models/obj/plane.obj").c_str(), (Config.ResourcesBasePath + "models/obj/plane_diffuse.png").c_str());
	propModelComponent->castShadow = Component::eShadowCastingType::Shadow;
	propModelComponent->receiveShadow = true;
	
	//Place player
	Actor = _Scene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };
	Actor->Position = Vector3{ 0.f,0.0f,0.f };
	Actor->Rotation = Vector3{ 0,0,0 };
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(0);
	animPlayerComponent->castShadow = Component::eShadowCastingType::Shadow;
	animPlayerComponent->receiveShadow = true;

	pDepthRenderer = new DepthRenderPass(sceneLight);
	pDepthRenderer->Create(_Scene);

	pShadowMapRenderer = new ShadowMapRenderPass(sceneLight, pDepthRenderer->shadowMap.depth.id);
	pShadowMapRenderer->Create(_Scene);
}

void Demo7PCFShadow::Update(float ElapsedSeconds)
{
	if (IsKeyDown(KEY_W)) {
		// Move player forward based on their rotation
		Actor->Position.x += sin(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
		Actor->Position.z += cos(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
	}

	if (IsKeyDown(KEY_S)) {
		// Move player backward based on their rotation
		Actor->Position.x -= sin(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
		Actor->Position.z -= cos(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
	}

	if (IsKeyDown(KEY_A)) {
		Actor->Rotation.y += 1;  // Rotate left
	}

	if (IsKeyDown(KEY_D)) {
		Actor->Rotation.y -= 1;  // Rotate right
	}

	Vector3 cameraPos = _Scene->GetMainCameraActor()->GetPosition();
	SetShaderValue(pShadowMapRenderer->shadowShader, pShadowMapRenderer->shadowShader.locs[SHADER_LOC_VECTOR_VIEW], &cameraPos, SHADER_UNIFORM_VEC3);
	float dt = GetFrameTime();
	if (IsKeyDown(KEY_J))
	{
		if (sceneLight->lightDir.x < 0.6f)
			sceneLight->lightDir.x += cameraSpeed * 60.0f * dt;
	}

	if (IsKeyDown(KEY_K))
	{
		if (sceneLight->lightDir.x > -0.6f)
			sceneLight->lightDir.x -= cameraSpeed * 60.0f * dt;
	}

	if (IsKeyDown(KEY_I))
	{
		if (sceneLight->lightDir.z < 0.6f)
			sceneLight->lightDir.z += cameraSpeed * 60.0f * dt;
	}

	if (IsKeyDown(KEY_M))
	{
		if (sceneLight->lightDir.z > -0.6f)
			sceneLight->lightDir.z -= cameraSpeed * 60.0f * dt;
	}

	if (IsKeyDown(KEY_O))
	{
		if (sceneLight->lightDir.y < 0.6f)
			sceneLight->lightDir.y += cameraSpeed * 60.0f * dt;
	}

	if (IsKeyDown(KEY_P))
	{
		if (sceneLight->lightDir.y > -0.6f)
			sceneLight->lightDir.y -= cameraSpeed * 60.0f * dt;
	}

	sceneLight->lightDir = Vector3Normalize(sceneLight->lightDir);
	pDepthRenderer->pLight->GetCamera3D()->position = Vector3Scale(sceneLight->lightDir, -100.0f);
	SetShaderValue(pShadowMapRenderer->shadowShader, pShadowMapRenderer->lightDirLoc, &sceneLight->lightDir, SHADER_UNIFORM_VEC3);
	__super::Update(ElapsedSeconds);  //allow update main camera
}

void Demo7PCFShadow::DrawOffscreen()
{
	pDepthRenderer->BeginShadowMap();
	pDepthRenderer->BeginScene();
	pDepthRenderer->Render();
	pDepthRenderer->EndScene();
	pDepthRenderer->EndShadowMap();
}

void Demo7PCFShadow::DrawFrame()
{
	pShadowMapRenderer->BeginScene();
	pShadowMapRenderer->Render();
	pShadowMapRenderer->EndScene();	
}

void Demo7PCFShadow::DrawGUI()
{
	Rectangle sr = { 0,0,1023, 1023 };
	Rectangle dr = { 0,0,255, 255 };
	Vector2 pos = { SCREEN_WIDTH - 300, 300 };
	DrawTextureEx(pDepthRenderer->shadowMap.depth, pos, 0, 0.25f, WHITE);
	DrawText("Use the I/J/K/M to move the light!", 10, 50, 30, WHITE);
	DrawText("Drag mouse with right click to rotate the camera!", 10, 90, 30, WHITE);
	DrawText("Use mouse wheel to zoom the camera", 10, 130, 30, WHITE);
}

void Demo7PCFShadow::OnCreateDefaultResources()
{
	//Use a true typeface for the default font
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}

void Demo7PCFShadow::OnConfigKnightApp()
{
	Config.EnableDefaultLight = false; //Disable default light	
	Config.EnableDefaultRenderPasses = false; //Disable default render passes, we will use ours.
}