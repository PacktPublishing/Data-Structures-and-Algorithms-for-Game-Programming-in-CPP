#include "Demo7Particle.h"

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#define RAYGUI_IMPLEMENTATION
#include "../../raylib/examples/core/raygui.h"

// Define particle count
#define PARTICLE_COUNT 500

//main particle origin position
int main(int argc, char* argv[])
{
	Demo7Particle* KnightDemo7Particle = new Demo7Particle();

	KnightDemo7Particle->Start();
	KnightDemo7Particle->GameLoop();

	delete KnightDemo7Particle;
	return 0;
}

// Initialize Knight Engine with a default scene, camera, and particle system
void Demo7Particle::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	//initialize global UI attributes
	GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

	pMainCamera = _Scene->CreateSceneObject<FlyThroughCamera>("Main Camera");
	pMainCamera->SetUp(Vector3{ 0.0f, 0.5f, 0.0f }, 10.0f, 0, 30, 45.0f, CAMERA_PERSPECTIVE);

	//Place player
	Actor = _Scene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };
	Actor->Position = Vector3{ 0.f,0.5f,0.f };
	Actor->Rotation = Vector3{ 0,0,0 };
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel("../../resources/models/gltf/robot.glb");
	animPlayerComponent->SetAnimation(6);

	// Set up particle system
	pParticle = Actor->CreateAndAddComponent<ParticleComponent>();
	pParticle->CreateFromFile("../../resources/textures/glow.png", PARTICLE_COUNT, fountainOrigin, Color{ 255, 150, 50, 255 }, Vector3{0,0,0});

}

// Update the scene
void Demo7Particle::Update(float ElapsedSeconds)
{
	if (IsKeyDown(KEY_W)) {
		// Move player forward based on their rotation
		Actor->Position.x += sinf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
		Actor->Position.z += cosf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_S)) {
		// Move player backward based on their rotation
		Actor->Position.x -= sinf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
		Actor->Position.z -= cosf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_A)) {
		Actor->Rotation.y += 1;  // Rotate left
	}
	if (IsKeyDown(KEY_D)) {
		Actor->Rotation.y -= 1;  // Rotate right
	}

	__super::Update(ElapsedSeconds);
}

// Draw the frame
void Demo7Particle::DrawFrame()
{
	DrawGrid(10, 1);
	__super::DrawFrame();
}

//Create default resources for the demo
void Demo7Particle::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx("../../resources/fonts/sparky.ttf", 32, 0, 0);
}
