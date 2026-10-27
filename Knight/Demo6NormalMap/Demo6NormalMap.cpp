#include "Demo6NormalMap.h"

#include "raymath.h"

#include <cmath>

//Main entry point for the application
int main(int argc, char* argv[])
{
	Demo6NormalMap* KnightDemo6NormalMap = new Demo6NormalMap();

	KnightDemo6NormalMap->Start();
	KnightDemo6NormalMap->GameLoop();

	delete KnightDemo6NormalMap;
	return 0;
}

// Start the demo, initializing the scene, camera, and loading textures and shaders
void Demo6NormalMap::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)

	pMainCamera = _Scene->CreateSceneObject<FlyThroughCamera>("Main Camera");
	pMainCamera->SetUp(Vector3{ 0.0f, 1.0f, 0.0f }, 8.0f, 0, 10, 60, CAMERA_PERSPECTIVE);

	diffuse = LoadTexture((Config.ResourcesBasePath + "textures/stonewall.png").c_str());  // Replace with your diffuse texture path
	normalMap = LoadTexture((Config.ResourcesBasePath + "textures/stonewall_n.png").c_str()); // Replace with your normal map texture path

	// Load shaders
	shader = LoadShader((Config.ResourcesBasePath + "shaders/glsl330/normalmap.vs").c_str(),
		(Config.ResourcesBasePath + "shaders/glsl330/normalmap.fs").c_str());

	_Scene->_CurrentRenderPass->Hints.pOverrideShader = &shader; // Set the shader to be used for rendering

	cubeActor = _Scene->CreateSceneObject<SceneActor>("Cube");
	cubeActor->Position = Vector3{ 3.0f, -1.0f, 0.0f };
	ModelComponent* modelComponent = cubeActor->CreateAndAddComponent<ModelComponent>();
	modelComponent->Load3DModel((Config.ResourcesBasePath + "models/obj/well.obj").c_str(), (Config.ResourcesBasePath + "models/obj/well_diffuse.png").c_str());
	for (int i = 0; i < modelComponent->GetModel()->materialCount; i++) {
		modelComponent->GetModel()->materials[i].shader = shader;
		modelComponent->GetModel()->materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = diffuse;
		modelComponent->GetModel()->materials[i].maps[MATERIAL_MAP_NORMAL].texture = normalMap;
	}
	for (int i = 0; i < modelComponent->GetModel()->meshCount; i++) {
		GenMeshTangents(&modelComponent->GetModel()->meshes[i]);
	}

	// Load a MD3 format model
	characterActor = _Scene->CreateSceneObject<SceneActor>("Character");
	characterActor->Scale = Vector3{ 2.0f, 2.0f, 2.0f };
	characterActor->Position = Vector3{ -3.0f, 0.0f, 0.0f };
	ModelComponent* animModelComponent = characterActor->CreateAndAddComponent<ModelComponent>();
	animModelComponent->Load3DModel((Config.ResourcesBasePath + "models/m3d/suzanne.m3d").c_str());
	animModelComponent->SetAnimation(10);
	for (int i = 0; i < animModelComponent->GetModel()->materialCount; i++) {
		animModelComponent->GetModel()->materials[i].shader = shader;
		animModelComponent->GetModel()->materials[i].maps[MATERIAL_MAP_DIFFUSE].texture = diffuse;
		animModelComponent->GetModel()->materials[i].maps[MATERIAL_MAP_NORMAL].texture = normalMap;
	}
	for (int i = 0; i < animModelComponent->GetModel()->meshCount; i++) {
		GenMeshTangents(&animModelComponent->GetModel()->meshes[i]);
	}

	// Get shader uniform locations
	lightPosLoc = GetShaderLocation(shader, "LightPosition_worldspace");	
	SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
	lightPowerLoc = GetShaderLocation(shader, "LightPower");
	SetShaderValue(shader, lightPowerLoc, &lightPower, SHADER_UNIFORM_FLOAT);
	lightColorLoc = GetShaderLocation(shader, "LightColor");
	SetShaderValue(shader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);
}

// Unload resources
void Demo6NormalMap::EndGame()
{
	UnloadTexture(diffuse);
	UnloadTexture(normalMap);
	UnloadShader(shader);
	__super::EndGame();
}

// Update light position and character rotation based on input
void Demo6NormalMap::Update(float ElapsedSeconds)
{
	if (IsKeyDown(KEY_W)) {
		lightPos.z += ElapsedSeconds;
	}
	if (IsKeyDown(KEY_S)) {
		// Move player backward based on their rotation
		lightPos.z -= ElapsedSeconds;
	}
	if (IsKeyDown(KEY_A)) {
		lightPos.x -= ElapsedSeconds;  // left
	}
	if (IsKeyDown(KEY_D)) {
		lightPos.x += ElapsedSeconds;  // right
	}

	SetShaderValue(shader, lightPosLoc, (void *) &lightPos, SHADER_UNIFORM_VEC3);

	Vector3 rot = characterActor->Rotation;
	rot.y	+= ElapsedSeconds * 20.0f;
	if (rot.y > 360.0f) {
		rot.y -= 360.0f;
	}	
	characterActor->Rotation = rot;
	cubeActor->Rotation = rot;

	__super::Update(ElapsedSeconds);
}

//Render scene and light source as a small sphere
void Demo6NormalMap::DrawFrame()
{
	__super::DrawFrame();

	// Draw light as a small sphere
	DrawSphere(lightPos, 0.1f, YELLOW);
}

//Render help text and debug information 
void Demo6NormalMap::DrawGUI()
{
	__super::DrawGUI();

	char buf[80];

	Vector3 v = pMainCamera->GetPosition();
	sprintf_s(buf, sizeof(buf), "cam: %f, %f, %f", v.x, v.y, v.z);
	DrawTextEx(_Font, buf, Vector2{ 100, 100 }, 40,4, WHITE);

	v = lightPos;
	sprintf_s(buf, sizeof(buf), "light: %f, %f, %f", v.x, v.y, v.z);
	DrawTextEx(_Font, buf, Vector2{ 100, 170 }, 40,4, WHITE);

	DrawTextEx(_Font, "Use WSAD to move light source.", Vector2{100, 50}, 40, 4, WHITE);
}

//Create default resources for the demo
void Demo6NormalMap::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}