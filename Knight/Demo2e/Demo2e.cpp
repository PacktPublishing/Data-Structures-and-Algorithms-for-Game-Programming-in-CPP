#include "Knight.h"
#include "Demo2e.h"

// Include the necessary headers for the UI functions of raylib
// Check https://github.com/raysan5/raygui for more details
#include "../../raylib/examples/core/raygui.h"

//Main entry point for the demo application
int main(int argc, char* argv[])
{
	Demo2e* KnightDemo2e = new Demo2e();

	KnightDemo2e->Start();
	KnightDemo2e->GameLoop();

	delete KnightDemo2e;
	return 0;
}

//Singleton instance of UIPopupManager
UIPopupManager* UIPopup::ui_manager = NULL;

//Initialize the demo class
void Demo2e::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();
	GuiSetFont(_Font);

	EnableCursor();

	Manager.Create();
}

//Check if the demo needs to open menu popup
void Demo2e::Update(float ElapsedSeconds)
{
	//process input
	__super::Update(ElapsedSeconds);

	if (!Manager.IsAnyPopupShown()) {
		if (IsKeyPressed(KEY_M)) {
			Manager.Show("Menu");
		}
	}

	Manager.Update(ElapsedSeconds);
}

//Render help text and UI popups
void Demo2e::DrawGUI()
{
	if (!Manager.IsAnyPopupShown()) {
		DrawText("Press M to open the in-game menu. Backspace key to go back to previous menu.", 10, 50, 40, WHITE);
	}
	else {
		DrawText("Press Backspace key to go back to previous menu.", 10, 50, 40, WHITE);
	}
	Manager.Draw();
}

//Load the default resources for the demo
void Demo2e::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx("../../resources/fonts/sparky.ttf", 32, 0, 0);
}
