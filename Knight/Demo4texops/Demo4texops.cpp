#include "Knight.h"

// Include the necessary headers for the UI functions of raylib
// Check https://github.com/raysan5/raygui for more details
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#define RAYGUI_IMPLEMENTATION
#include "Demo4texops.h"

//The main application function
int main(int argc, char* argv[])
{
	Demo4TexOps* KnightDemo4TexOps = new Demo4TexOps();

	KnightDemo4TexOps->Start();
	KnightDemo4TexOps->GameLoop();

	delete KnightDemo4TexOps;
	return 0;
}

//Create necessary reosurces
void Demo4TexOps::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

	InitEntities();
}

//Update with current demo entity
void Demo4TexOps::Update(float ElapsedSeconds)
{
	__super::Update(ElapsedSeconds);

	if (IsKeyPressed(KEY_ENTER) == true || IsMouseButtonPressed(0)== true) {
		(*it)->Release();
		++it;
		return;
	}

	if ((*it)->isReady == true)
		(*it)->Update(ElapsedSeconds);
}

//Draw the GUI for the current demo
void Demo4TexOps::DrawGUI()
{
	Rectangle r = {200,800,1520,250};
	Rectangle r1 = { 200,830,1520,250};
	Rectangle r2 = { 200, 1000, 1520, 50};

	__super::DrawGUI();

	if (it == demos.end()) {
		ExitGameLoop();
		return;
	}

	if ((*it)->isReady == true) {
		(*it)->Draw2D();
	} else {
		// we need to create resources for this demo first
		DrawText("Loading demo ...", 600, 500, 50, WHITE);
		(*it)->Create();
	}

	GuiWindowBox(r, (*it)->title.c_str());
	GuiDrawText((*it)->description.c_str(), r1, TEXT_ALIGN_CENTER, BLUE);
	GuiDrawText("Press 'Enter' or click mouse left button to continue...", r2, TEXT_ALIGN_CENTER, RED);
}

//Initialize all the demoscene entities
void Demo4TexOps::InitEntities()
{	
	demos.push_back(new SimpleDrawTextureDemo());
	demos.push_back(new CompressTextureDemo());
	demos.push_back(new LRUTextureCacheDemo(3));
	demos.push_back(new DrawPartialRotateDemo());
	demos.push_back(new ColorBlendingDemo());
	demos.push_back(new SmoothColorBlendingDemo()); 
	demos.push_back(new SceneTransitionDemo());
	demos.push_back(new GlowDemo());
	demos.push_back(new AnimatedTexDemo());
	demos.push_back(new NPatchDemo());

	it = demos.begin();
}

//Create default resources, such as fonts, shaders, etc.
void Demo4TexOps::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx("../../resources/fonts/sparky.ttf", 32, 0, 0);
	GuiSetFont(_Font);
}
