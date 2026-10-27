#include "Knight.h"
#include "Demo2f.h"

#include <time.h>

// Include the necessary headers for the UI functions of raylib
// Check https://github.com/raysan5/raygui for more details
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#define RAYGUI_IMPLEMENTATION
#include "../../raylib/examples/core/raygui.h"

//Main function of the application
int main(int argc, char* argv[])
{
	Demo2f* KnightDemo2f = new Demo2f();

	KnightDemo2f->Start();
	KnightDemo2f->GameLoop();

	delete KnightDemo2f;
	return 0;
}

//Console messages
std::list<std::string> Demo2f::messages;

//Gameplay state
GameResult Demo2f::_gameOver = InProgress;

//Initialize the game
void Demo2f::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	//initialize global UI attributes
	GuiSetFont(_Font);
	GuiSetStyle(PROGRESSBAR, TEXT_COLOR_NORMAL, 0xffffffff);
	GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

	srand((unsigned int) time(NULL));

	//Prepare a camera
	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetPosition(Vector3{ 60, 30, 60 });
	camera->SetLookAtPosition(Vector3{0, 3, 0});

	InitEntities();

	Log("Start game!");
}

void Demo2f::Update(float ElapsedSeconds)
{
	//process input
	player->Update(ElapsedSeconds);
	enemy->Update(ElapsedSeconds);
	terrain->Update(ElapsedSeconds);
	__super::Update(ElapsedSeconds);
}

//Render gameplay status
void Demo2f::DrawGUI()
{
	player->DrawGUI();
	enemy->DrawGUI();

	switch (_gameOver)
	{
		case InProgress:
		{
			int line = 0;
			for (const auto& msg : messages) {
				DrawText(msg.c_str(), 150, 150 + 40 * line, 32, WHITE);
				++line;
			}
			break;
		}
		case YouWin:
		{
			Rectangle r;
			int activeButton = 0;
			int result = GuiMessageBox(CenterRectangle(r, 400, 400),
				"#191#Congrate!", "You Win!", "Nice!", &activeButton);
			if (result > 0 || IsKeyPressed(KEY_ENTER))
				exit(0);
			break;
		}
		case YouLose:
		{
			Rectangle r;
			int activeButton = 0;
			int result = GuiMessageBox(CenterRectangle(r, 400, 400),
				"#191#Oh No!", "You Lose!", "Okay", &activeButton);
			if (result > 0 || IsKeyPressed(KEY_ENTER))
				exit(0);
			break;
		}
	}

}

//Initialize entities in the scene
void Demo2f::InitEntities()
{
	terrain = new TerrainEntity();
	terrain->Create(_Scene, NULL);

	player = new PlayerEntity();
	player->Create(_Scene, NULL);
	
	enemy = new EnemyEntity();
	enemy->Create(_Scene, NULL);

	player->SetTarget(enemy);
	enemy->SetTarget(player);
}

void Demo2f::GameOver(GameResult gr)
{
	_gameOver = gr;
}

void Demo2f::Log(const std::string& message) {
	if (messages.size() >= 10) {
		messages.pop_front();  // Remove the oldest message
	}
	messages.push_back(message);  // Add the new message
}

//Load the default resources for the demo
void Demo2f::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
