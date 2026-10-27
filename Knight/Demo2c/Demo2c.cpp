//Demo2c - Using std::vector to manage multiple enemies in a scene
#include "Knight.h"
#include "Demo2c.h"

//Main entry point for the application
int main(int argc, char* argv[])
{
	Demo2c* KnightDemo2c = new Demo2c();

	KnightDemo2c->Start();
	KnightDemo2c->GameLoop();

	delete KnightDemo2c;
	return 0;
}

//Initialize the demo
void Demo2c::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	//Prepare a camera
	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3 { 60, 30, 60 });
	camera->CameraMode = CameraMode::CAMERA_THIRD_PERSON;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0, 10, 0 });

	InitEntities();
}

//Update the demo each frame
void Demo2c::Update(float ElapsedSeconds)
{
	//process input
	CheckDefeatEnemy();
	player->Update(ElapsedSeconds);
	for(EnemyEntity& enemy : enemies) {
		enemy.Update(ElapsedSeconds);
	}		
	terrain->Update(ElapsedSeconds);
	__super::Update(ElapsedSeconds);
}

//Initialize entities in the scene
void Demo2c::InitEntities()
{
	terrain = new TerrainEntity();
	terrain->Create(_Scene, nullptr);

	player = new PlayerEntity();
	player->Create(_Scene, nullptr);

	for (int i = 0; i < 3; i++) {
		EnemyEntity* ee = new EnemyEntity();
		ee->Create(_Scene, nullptr);
		enemies.push_back(*ee);
	}

}

//Check if the player has defeated any enemies
void Demo2c::CheckDefeatEnemy()
{
	if (IsKeyPressed(KEY_ONE) && enemies[0].HP > 0)
		enemies[0].Die();
	if (IsKeyPressed(KEY_TWO) && enemies[1].HP > 0)
		enemies[1].Die();
	if (IsKeyPressed(KEY_THREE) && enemies[2].HP > 0)
		enemies[2].Die();	
}

//Render help text on the screen
void Demo2c::DrawGUI()
{
	DrawText("Press 1 to defeat 1st enemy, 2 to defeat 2nd enemy, 3 to defeat 3rd enemy!", 10, 50, 40, WHITE);
	DrawText("Rotate camera angle by mouse or arrow keys.", 10, 100, 40, WHITE);
	DrawText("Move camera by w/s/a/d keys.", 10, 150, 40, WHITE);
	DrawText("Zoom Camera by mouse wheel.", 10, 200, 40, WHITE);
}

//Load default resources for the demo
void Demo2c::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
