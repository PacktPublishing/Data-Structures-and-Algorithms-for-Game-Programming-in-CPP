#include "Demo53PV.h"

#include <cmath>

//main function to start the demo
int main(int argc, char* argv[])
{
	Demo53PV* KnightDemo53PV = new Demo53PV();

	KnightDemo53PV->Start();
	KnightDemo53PV->GameLoop();

	delete KnightDemo53PV;
	return 0;
}

//Create camera and player actor, set up initial positions and animations
void Demo53PV::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	ChaseCamera = _Scene->CreateSceneObject<FollowUpCamera>("Chase Camera");

	//Place player
	Actor = _Scene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };
	Actor->Position = Vector3{ 0.f,0.5f,0.f };
	Actor->Rotation = Vector3{ 0,180.0f,0 };
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(6);

	ChaseCamera->SetUp(Actor, 45.0f, 5.0f, CAMERA_PERSPECTIVE);
}

// Update player position and rotation based on input
void Demo53PV::Update(float ElapsedSeconds)
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

	__super::Update(ElapsedSeconds);
}

// Draw the frame, including grid and player movement visualization
void Demo53PV::DrawFrame()
{
	__super::DrawFrame();

	// Draw grid for better spatial reference
	DrawGrid(10, 1.0f);

	//Draw a line from the player actor's position in the direction they are facing
	Vector3 forward = {
		Actor->Position.x + sin(DegreesToRadians(Actor->Rotation.y)),
		Actor->Position.y,
		Actor->Position.z + cos(DegreesToRadians(Actor->Rotation.y))
	};
	DrawLine3D(Actor->Position, forward, GREEN);

}

// Draw GUI elements to provide instructions to the user
void Demo53PV::DrawGUI()
{
	__super::DrawGUI();

	DrawText("Use W/A/S/D to move player", 10, 50, 40, WHITE);
	DrawText("Hold right mouse button and move mouse to rotate camera", 10, 100, 40, WHITE);
	DrawText("Use mouse wheel to adjust camera distance", 10, 150, 40, WHITE);
}

// Load default resources such as fonts
void Demo53PV::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
