#include "Demo6LightShader.h"

#include "raymath.h"

#include <cmath>

//main application entry point
int main(int argc, char* argv[])
{
	Demo6LightShader* KnightDemo6Billboard = new Demo6LightShader();

	KnightDemo6Billboard->Start();
	KnightDemo6Billboard->GameLoop();

	delete KnightDemo6Billboard;
	return 0;
}

// Start the demo, initializing the scene, camera, player, and light
void Demo6LightShader::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	// Create lights
	FlyThroughCamera* pMainCamera = _Scene->CreateSceneObject<FlyThroughCamera>("Main Camera");
	pMainCamera->SetUp(Vector3{ 0, 0.5, 0 }, 20.0f, 0, 40.0f, 45.0f, CAMERA_PERSPECTIVE);

	pLight = _Scene->CreateSceneObject<SceneLight>("Light");
	pLight->Scale = Vector3{ 1,1, 1 };
	pLight->Position = Vector3{ 3.0f, 5.0f, -3.0f };
	pLight->Rotation = Vector3{ 0,0,0 };
	pLight->Create();

	//Place player
	pPlayer = _Scene->CreateSceneObject<SceneActor>("player");
	pPlayer->Scale = Vector3{ 1,1,1 };
	pPlayer->Position = Vector3{ 0.f,0.5f,0.f };
	pPlayer->Rotation = Vector3{ 0,0,0 };
	ModelComponent* animPlayerComponent = pPlayer->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(6);
}

// Update player position and light position based on input
void Demo6LightShader::Update(float ElapsedSeconds)
{
	if (IsKeyDown(KEY_W)) {
		// Move player forward based on their rotation
		pPlayer->Position.x += sin(DegreesToRadians(pPlayer->Rotation.y)) * 0.1f;
		pPlayer->Position.z += cos(DegreesToRadians(pPlayer->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_S)) {
		// Move player backward based on their rotation
		pPlayer->Position.x -= sin(DegreesToRadians(pPlayer->Rotation.y)) * 0.1f;
		pPlayer->Position.z -= cos(DegreesToRadians(pPlayer->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_A)) {
		pPlayer->Rotation.y += 1;  // Rotate left
	}
	if (IsKeyDown(KEY_D)) {
		pPlayer->Rotation.y -= 1;  // Rotate right
	}

	// Update light position for animation (simple circular motion)
	pLight->Position.x = 4.0f * cosf((float)GetTime());
	pLight->Position.z = 4.0f * sinf((float)GetTime());
	pLight->lightDirection = Vector3Normalize(Vector3Subtract(Vector3{ 0,0,0 }, pLight->Position));
	__super::Update(ElapsedSeconds);
}

// Draw the frame, including the player and light
void Demo6LightShader::DrawFrame()
{
	__super::DrawFrame();

	DrawSphereWires(pLight->Position, 0.2f, 8, 8, WHITE);

	// Draw grid for better spatial reference
	DrawGrid(10, 1.0f);
}

// Load default resources like fonts, textures, etc.
void Demo6LightShader::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}