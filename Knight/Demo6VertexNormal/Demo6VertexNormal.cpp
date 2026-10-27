#include "Demo6VertexNormal.h"

#include "raymath.h"

#define RLIGHTS_IMPLEMENTATION
#include "../../raylib/examples/models/rlights.h"

#define GLSL_VERSION            330

#include <cmath>

//main application function
int main(int argc, char* argv[])
{
	Demo6VertexNormal* KnightDemo6VertexNormal = new Demo6VertexNormal();

	KnightDemo6VertexNormal->Start();
	KnightDemo6VertexNormal->GameLoop();

	delete KnightDemo6VertexNormal;
	return 0;
}

//create lights and SceneActor
void Demo6VertexNormal::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	SetConfigFlags(FLAG_MSAA_4X_HINT);  // Enable Multi Sampling Anti Aliasing 4x (if available)

	shader = LoadShader(TextFormat((Config.ResourcesBasePath + "shaders/glsl%i/lighting.vs").c_str(), GLSL_VERSION),
		TextFormat((Config.ResourcesBasePath + "shaders/glsl%i/lighting.fs").c_str(), GLSL_VERSION));

	_Scene->_CurrentRenderPass->Hints.pOverrideShader = &shader;

	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
	ambientLoc = GetShaderLocation(shader, "ambient");
	const float ac[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
	SetShaderValue(shader, ambientLoc, ac, SHADER_UNIFORM_VEC4);

	// Create lights	
	lights[0] = CreateLight(LIGHT_POINT, Vector3{ -4, 6, -4 }, Vector3Zero(), YELLOW, shader);
	lights[1] = CreateLight(LIGHT_POINT, Vector3{ 4, 6, 4 }, Vector3Zero(), RED, shader);
	lights[2] = CreateLight(LIGHT_POINT, Vector3{ -4, 6, 4 }, Vector3Zero(), GREEN, shader);
	lights[3] = CreateLight(LIGHT_POINT, Vector3{ 4, 6, -4 }, Vector3Zero(), BLUE, shader);

	pMainCamera = _Scene->CreateSceneObject<FlyThroughCamera>("Main Camera");
	pMainCamera->SetUp(Vector3{ 0,4,0 }, 12.0f, 0, 30.0f, 45.0f, CAMERA_PERSPECTIVE);

	//Place player model with face normals
	FNActor = _Scene->CreateSceneObject<SceneActor>("Player");
	FNActor->Position = Vector3{ 3.0f,0.0f,0.f };
	FNActor->Rotation = Vector3{ 0,0,0 };
	ModelComponent* FNPlayerModel = FNActor->CreateAndAddComponent<ModelComponent>();
	FNPlayerModel->_AlwaysSmoothNormal = false;
	FNPlayerModel->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	FNPlayerModel->SetAnimation(6);
	for (int i = 0; i < FNPlayerModel->GetModel()->materialCount; i++) {
		FNPlayerModel->GetModel()->materials[i].shader = shader;
	}

	//Place player model with vertex normals
	VNActor = _Scene->CreateSceneObject<SceneActor>("Player");
	VNActor->Position = Vector3{ -3.0f,0.0f,0.f };
	VNActor->Rotation = Vector3{ 0,0,0 };
	ModelComponent* VNPlayerModel = VNActor->CreateAndAddComponent<ModelComponent>();
	VNPlayerModel->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	VNPlayerModel->SetAnimation(6);
	for (int i = 0; i < VNPlayerModel->GetModel()->materialCount; i++) {
		VNPlayerModel->GetModel()->materials[i].shader = shader;
	}
}

//Process user input to move SceneActor and toggle lights
void Demo6VertexNormal::Update(float ElapsedSeconds)
{
	float cameraPos[3] = { pMainCamera->GetPosition().x, pMainCamera->GetPosition().y, pMainCamera->GetPosition().z };
	SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

	// Check key inputs to enable/disable lights
	if (IsKeyPressed(KEY_Y)) { lights[0].enabled = !lights[0].enabled; }
	if (IsKeyPressed(KEY_R)) { lights[1].enabled = !lights[1].enabled; }
	if (IsKeyPressed(KEY_G)) { lights[2].enabled = !lights[2].enabled; }
	if (IsKeyPressed(KEY_B)) { lights[3].enabled = !lights[3].enabled; }

	for (int i = 0; i < MAX_LIGHTS; i++) {
		lights[i].position = RotateAroudnY(lights[i].position, 20.0f * ElapsedSeconds);
		UpdateLightValues(shader, lights[i]);
	}

	__super::Update(ElapsedSeconds);
}

//Render the frame and light sources as colored spheres
void Demo6VertexNormal::DrawFrame()
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
void Demo6VertexNormal::DrawGUI()
{
	DrawText("Press Y/G/R/B to toggle Yellow/Green/Red/Blue light", 10, 50, 40, WHITE);
	DrawText("Both models are load by raylib's LoadModelAnimations().", 10, 90, 40, WHITE);
	DrawText("The left model use recaculated vertex noramls to get smooth surface shading.", 10, 130, 40, WHITE);
	DrawText("The right model use default normals loaded from the model as it is.", 10, 170, 40, WHITE);
	DrawText("To learn how the algorithm works, check out the accompanying mini cookbook.", 10, 230, 40, YELLOW);
}

//Create default resources for the demo
void Demo6VertexNormal::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}

Vector3 Demo6VertexNormal::RotateAroudnY(const Vector3& v, float degrees)
{
	float rad = degrees * DEG2RAD;     // Raylib provides DEG2RAD
	float c = cosf(rad);
	float s = sinf(rad);

	// x' =  x*c + z*s
	// y' =  y
	// z' = -x*s + z*c
	return { v.x * c + v.z * s,
			 v.y,
			-v.x * s + v.z * c };
}
