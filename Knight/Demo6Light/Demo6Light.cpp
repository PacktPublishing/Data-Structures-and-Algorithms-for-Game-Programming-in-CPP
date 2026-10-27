#include "Demo6Light.h"

#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "../../raylib/examples/models/rlights.h"

#define GLSL_VERSION            330

#include <cmath>

//main application function
int main(int argc, char* argv[])
{
	Demo6Light* KnightDemo6Billboard = new Demo6Light();

	KnightDemo6Billboard->Start();
	KnightDemo6Billboard->GameLoop();

	delete KnightDemo6Billboard;
	return 0;
}

//create lights and SceneActor
void Demo6Light::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)

	shader = LoadShader(TextFormat("../../resources/shaders/glsl%i/lighting.vs", GLSL_VERSION),
		TextFormat("../../resources/shaders/glsl%i/lighting.fs", GLSL_VERSION));

	_Scene->_CurrentRenderPass->Hints.pOverrideShader = &shader;

	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	ambientLoc = GetShaderLocation(shader, "ambient");
	const float ac[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	SetShaderValue(shader, ambientLoc, ac , SHADER_UNIFORM_VEC4);

	// Create lights	
	lights[0] = CreateLight(LIGHT_POINT, Vector3 { -2, 1, -2 }, Vector3Zero(), YELLOW, shader);
	lights[1] = CreateLight(LIGHT_POINT, Vector3 { 2, 1, 2 }, Vector3Zero(), RED, shader);
	lights[2] = CreateLight(LIGHT_POINT, Vector3 { -2, 1, 2 }, Vector3Zero(), GREEN, shader);
	lights[3] = CreateLight(LIGHT_POINT, Vector3 { 2, 1, -2 }, Vector3Zero(), BLUE, shader);

	pMainCamera = _Scene->CreateSceneObject<FlyThroughCamera>("Main Camera");
	pMainCamera->SetUp(Vector3{0,0,0}, 6.0f, 0, 30.0f, 45.0f, CAMERA_PERSPECTIVE);

	//Place player
	actor = _Scene->CreateSceneObject<SceneActor>("Player");
	actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };
	actor->Position = Vector3{ 0.f,0.5f,0.f };
	actor->Rotation = Vector3{ 0,0,0 };
	ModelComponent* animPlayerComponent = actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel("../../resources/models/gltf/robot.glb");
	animPlayerComponent->SetAnimation(6);
	for (int i = 0; i < animPlayerComponent->GetModel()->materialCount; i++) {
		animPlayerComponent->GetModel()->materials[i].shader = shader;
	}

	SceneActor *pTerrain = _Scene->CreateSceneObject<SceneActor>("Terrain");
	pTerrain->Position = Vector3{ 0, -3.5f, 0 };
	pTerrain->Scale = Vector3{ 1, 1, 1 };
	ModelComponent* animEnemyComponent = pTerrain->CreateAndAddComponent<ModelComponent>();
	animEnemyComponent->Load3DModel("../../resources/models/obj/bridge.obj");
	Image image = LoadImage("../../resources/models/obj/bridge_diffuse.png");
	Texture2D tex = LoadTextureFromImage(image);
	UnloadImage(image);
	animEnemyComponent->GetModel()->materials[0].maps[0].texture = tex;
	animEnemyComponent->GetModel()->materials[0].shader = shader;
}

//Process user input to move SceneActor and toggle lights
void Demo6Light::Update(float ElapsedSeconds)
{
	if (IsKeyDown(KEY_W)) {
		// Move player forward based on their rotation
		actor->Position.x += sin(DegreesToRadians(actor->Rotation.y)) * 0.1f;
		actor->Position.z += cos(DegreesToRadians(actor->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_S)) {
		// Move player backward based on their rotation
		actor->Position.x -= sin(DegreesToRadians(actor->Rotation.y)) * 0.1f;
		actor->Position.z -= cos(DegreesToRadians(actor->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_A)) {
		actor->Rotation.y += 1;  // Rotate left
	}
	if (IsKeyDown(KEY_D)) {
		actor->Rotation.y -= 1;  // Rotate right
	}

	float cameraPos[3] = { pMainCamera->GetPosition().x, pMainCamera->GetPosition().y, pMainCamera->GetPosition().z};
	SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

	// Check key inputs to enable/disable lights
	if (IsKeyPressed(KEY_Y)) { lights[0].enabled = !lights[0].enabled; }
	if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
	if (IsKeyPressed(KEY_G)) { lights[2].enabled = !lights[2].enabled; }
	if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }

	for (int i = 0; i < MAX_LIGHTS; i++) UpdateLightValues(shader, lights[i]);

	__super::Update(ElapsedSeconds);
}

//Render the frame and light sources as colored spheres
void Demo6Light::DrawFrame()
{
	__super::DrawFrame();
	// Draw spheres to show where the lights are
	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		if (lights[i].enabled) DrawSphereEx(lights[i].position, 0.2f, 8, 8, lights[i].color);
		else DrawSphereWires(lights[i].position, 0.2f, 8, 8, ColorAlpha(lights[i].color, 0.3f));
	}
}

//Render help text on the screen
void Demo6Light::DrawGUI()
{
	DrawText("Press Y/G/R/B to toggle Yellow/Green/Red/Blue light", 10, 50, 40, WHITE);
}

//Create default resources for the demo
void Demo6Light::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx("../../resources/fonts/sparky.ttf", 32, 0, 0);
}
