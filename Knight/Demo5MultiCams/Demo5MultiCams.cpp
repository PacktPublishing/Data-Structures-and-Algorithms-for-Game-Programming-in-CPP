#include "Demo5MultiCams.h"

#include <cmath>

//Main application function
int main(int argc, char* argv[])
{
	Demo5MultiCams* KnightDemo5MultiCams = new Demo5MultiCams();

	KnightDemo5MultiCams->Start();
	KnightDemo5MultiCams->GameLoop();

	delete KnightDemo5MultiCams;
	return 0;
}

//Create cameras for both views and initialize SceneActors
void Demo5MultiCams::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	// Create camera for both render targets
	pChaseCamera = new FollowUpCamera(_Scene, "Chase Camera", false);
	pTopDownCamera = new TopDownCamera(_Scene, "Map Camera", false);

	//Place player
	Actor = _Scene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };
	Actor->Position = Vector3{ 0.f,0.1f,0.f };
	Actor->Rotation = Vector3{ 0,90,0 };
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(6);
	Actor->AddComponent(animPlayerComponent);

	// Setup view 1 camera and screen	
	pChaseCamera->SetUp(Actor, 45.0f, 5.0f, CAMERA_PERSPECTIVE);
	pChaseCamera->processMouseInput = false;

	//Setup view 2 camera and screen
	pTopDownCamera->SetUp(Vector3{ -3.0f, 3.0f, 0 }, Actor->Position, 45.0f, CAMERA_PERSPECTIVE);

	//Create render textures
	ChaseCamRT = LoadRenderTexture(SCREEN_WIDTH / 2, SCREEN_HEIGHT);
	TopDownCamRT = LoadRenderTexture(SCREEN_WIDTH / 2, SCREEN_HEIGHT);

	splitScreenRect = { 0.0f, 0.0f, (float)ChaseCamRT.texture.width, (float)-ChaseCamRT.texture.height };
}

//Process player input and update camera
void Demo5MultiCams::Update(float ElapsedSeconds)
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
		// Rotate left
		Actor->Rotation.y += 1;  
	}
	if (IsKeyDown(KEY_D)) {
		// Rotate right
		Actor->Rotation.y -= 1;  
	}

	//Update cameras
	pChaseCamera->Update(ElapsedSeconds);
	pTopDownCamera->SetLookAtPosition(Actor->Position);
	pTopDownCamera->Update(ElapsedSeconds);

	//Update the scene
	__super::Update(ElapsedSeconds);
}

//Render both views
void Demo5MultiCams::DrawOffscreen()
{
	// Draw Player1 view to the render texture
	BeginTextureMode(ChaseCamRT);
	ClearBackground(DARKBLUE);

	BeginMode3D(*pChaseCamera->GetCamera3D());
	DrawGameWorld(pChaseCamera);
	EndMode3D();

	DrawRectangle(0, 0, GetScreenWidth() / 2, 40, Fade(RAYWHITE, 0.8f));
	DrawText("The 3rd person camera view", 10, 60, 40, YELLOW);

	EndTextureMode();

	// Draw Player2 view to the render texture
	BeginTextureMode(TopDownCamRT);
	ClearBackground(DARKPURPLE);

	BeginMode3D(*pTopDownCamera->GetCamera3D());
	DrawGameWorld(pTopDownCamera);
	EndMode3D();

	DrawRectangle(0, 0, GetScreenWidth() / 2, 40, Fade(RAYWHITE, 0.8f));
	DrawText("The top down camera view", 10, 60, 40, YELLOW);

	EndTextureMode();
}

void Demo5MultiCams::DrawFrame()
{
	//All the rendering is done in the offscreen render targets
	//So here we do not need to draw anything
	//We will render the offscreen textures in DrawGUI()
}

//Draw on-screen instructions
void Demo5MultiCams::DrawGUI()
{
	__super::DrawGUI();

	DrawTextureRec(ChaseCamRT.texture, splitScreenRect, Vector2{ 0, 0 }, WHITE);
	DrawTextureRec(TopDownCamRT.texture, splitScreenRect, Vector2{ SCREEN_WIDTH / 2.0f, 0 }, WHITE);

	DrawRectangle(GetScreenWidth() / 2 - 2, 0, 4, GetScreenHeight(), LIGHTGRAY);

	DrawText("Use WSAD to move player character..", 10, 150, 40, WHITE);
	DrawText("Use mouse wheel to zoom and right-click drag to rotate.", 10, 200, 40, WHITE);
}

//Render scene
void Demo5MultiCams::DrawGameWorld(SceneCamera *pCam)
{
	//allow Knight to draw the scene graph, now contains the player actor
	Knight::DrawFrame();   
	//Also draw a grid for reference
	DrawGrid(10, 1);
}

// This function is called when the engine is ready to create default resources like fonts, textures, etc.
void Demo5MultiCams::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);

}

void Demo5MultiCams::OnConfigKnightApp()
{
	Config.WindowTitle = "Demo 5 - Multiple Cameras";
}