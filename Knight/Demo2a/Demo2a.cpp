//Demo2a - Working with Entities and Knight
#include "Knight.h"
#include "Demo2a.h"

//main entry point for the application
int main(int argc, char* argv[])
{
	Demo2a* KnightDemo2a = new Demo2a();

	KnightDemo2a->Start();
	KnightDemo2a->GameLoop();

	delete KnightDemo2a;
	return 0;
}

// This function is called to initialize the game state, including the camera and entities.
void Demo2a::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	//Prepare a built-in camera and set its properties
	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3 { 60, 30, 60 });
	camera->CameraMode = CameraMode::CAMERA_THIRD_PERSON;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{0, 3, 0});

	//Create entities, refer to the introduction of Entity in Chapter 2.
	InitEntities();
}

// This function is called to update the game state, including the entities and the camera.
void Demo2a::Update(float ElapsedSeconds)
{
	//Update behavior/gameplay logic of these entities
	player->Update(ElapsedSeconds);
	enemy->Update(ElapsedSeconds);
	prop->Update(ElapsedSeconds);
	terrain->Update(ElapsedSeconds);

	//Update rendering settings of all SceneActors and camera position 
	__super::Update(ElapsedSeconds);
}

// This function is called to draw the frame, including the entities and the camera.
// There is nothing to do in this demo, just put here as an example.
// In your real application, you don't need to override this function unless you have special rendering requirements.
void Demo2a::DrawFrame()
{
	__super::DrawFrame();
}

// This function is called to initialize the entities in the scene.
void Demo2a::InitEntities()
{
	//First create a terrain entity, which is the ground of the scene.
	terrain = new TerrainEntity();
	terrain->Create(_Scene, NULL);

	//Create a player and an enemy entity on the terrain
	player = new PlayerEntity();
	player->Create(_Scene, terrain);

	//Create an prop Eneity 
	prop = new PropEntity();
	prop->Create(_Scene, terrain);

	//Create an emeny Entity
	enemy = new EnemyEntity();
	enemy->Create(_Scene, terrain);	
}

// This function is called to draw the GUI elements on the screen.
void Demo2a::DrawGUI()
{
	DrawText("Rotate camera angle by mouse or arrow keys.", 10, 50, 40, WHITE);
	DrawText("Move camera by w/s/a/d keys.", 10, 100, 40, WHITE);
	DrawText("Zoom Camera by mouse wheel.", 10, 150, 40, WHITE);
}

// This function is called when the application is created.
// It is used to load default resources such as fonts.
void Demo2a::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	//Loads a better TrueType font to display text information on the screen
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}