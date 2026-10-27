//Demo2d - Using std::list to handle multiple enemies
#include "Knight.h"
#include "Demo2d.h"

//Main entry point for the application
int main(int argc, char* argv[])
{
	Demo2d* KnightDemo2d = new Demo2d();

	KnightDemo2d->Start();
	KnightDemo2d->GameLoop();

	delete KnightDemo2d;
	return 0;
}

//Load resources and initialize the demo
void Demo2d::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	//Prepare a camera
	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3 { 60, 30, 60 });
	camera->CameraMode = CameraMode::CAMERA_THIRD_PERSON;
	camera->ShowCursor = false;
	camera->SetLookAtPosition( Vector3{ 0, 10, 0 });

	InitEntities();
}

//Update the demo state, read and process input, update gameplay logic in entities
void Demo2d::Update(float ElapsedSeconds)
{
	//process input
	CheckDefeatEnemy();
	player->Update(ElapsedSeconds);
	for (EnemyEntity& enemy : enemies) {
		enemy.Update(ElapsedSeconds);
	}
	terrain->Update(ElapsedSeconds);
	__super::Update(ElapsedSeconds);
}

//Initialize entities in the scene
void Demo2d::InitEntities()
{
	terrain = new TerrainEntity();
	terrain->Create(_Scene, NULL);

	player = new PlayerEntity();
	player->Create(_Scene, NULL);

	for (int i = 0; i < 3; i++) {
		EnemyEntity* ee = new EnemyEntity();
		ee->Create(_Scene, NULL);
		enemies.push_back(*ee);
	}

}

//Check if the player has defeated an enemy
void Demo2d::CheckDefeatEnemy()
{
	list<EnemyEntity>::iterator it = enemies.begin();	
	if (IsKeyPressed(KEY_ONE) && it->HP > 0)
		it->Die();
	++it;
	if (IsKeyPressed(KEY_TWO) && it->HP > 0)
		it->Die();
	++it;
	if (IsKeyPressed(KEY_THREE) && it->HP > 0)
		it->Die();	
}

//Render help and debug text on the screen
void Demo2d::DrawGUI()
{
	DrawText("Press 1 to defeat 1st enemy, 2 to defeat 2nd enemy, 3 to defeat 3rd enemy!", 10, 50, 40, WHITE);
	DrawText("Rotate camera angle by mouse or arrow keys.", 10, 100, 40, WHITE);
	DrawText("Move camera by w/s/a/d keys.", 10, 150, 40, WHITE);
	DrawText("Zoom Camera by mouse wheel.", 10, 200, 40, WHITE);

	list<EnemyEntity>::iterator it = enemies.begin();
	for (int i = 0; i < enemies.size(); i++) {
		if (it->respawnInterval > 0)
			DrawText(TextFormat("Enemy %d will be respawn in %1.2f seconds.", i + 1, it->respawnInterval), 10, 250 + i * 50, 40, WHITE);
		++it;
	}
}

//Load the default resources for the demo
void Demo2d::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
