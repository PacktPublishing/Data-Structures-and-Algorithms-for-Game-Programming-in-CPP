#include "Demo2eUI.h"

// Include the necessary headers for the UI functions of raylib
// Check https://github.com/raysan5/raygui for more details
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 4996)
#define RAYGUI_IMPLEMENTATION
#include "../../raylib/examples/core/raygui.h"

#include "KnightUtils.h"

void InGameDialogue::Create(UIPopupManager* uim)
{
	__super::Create(uim);
}

void InGameDialogue::Update(float interval)
{
	if (IsKeyPressed(KEY_BACKSPACE)) {
		ui_manager->Close();
	}
	if (IsKeyPressed(KEY_S)) {
		ui_manager->Show(ui_manager->GetPopup("Settings"));
	}
}

void InGameDialogue::Draw()
{
	Rectangle r;
	if (GuiWindowBox(CenterRectangle(r, 800, 600), "In Game Menu") == 1)
	{
		ui_manager->Close();
		return;
	}

	if (GuiButton(CenterRectangle(r, 300, 60), "Settings"))
		ui_manager->Show(ui_manager->GetPopup("Settings"));
}

void SettingsDialogue::Create(UIPopupManager* uim)
{
	__super::Create(uim);
}

void SettingsDialogue::Update(float interval)
{
	if (IsKeyPressed(KEY_BACKSPACE)) {
		ui_manager->Close();
	}
	if (IsKeyPressed(KEY_V)) {
		ui_manager->Show(ui_manager->GetPopup("Volume"));
	}
}

void SettingsDialogue::Draw()
{
	Rectangle r;
	if (GuiWindowBox(CenterRectangle(r, 700, 400), "Settings") == 1)
	{
		ui_manager->Close();
		return;
	}
	if (GuiButton(CenterRectangle(r, 300, 60), "Sound"))
		ui_manager->Show(ui_manager->GetPopup("Volume"));
}

SoundDialogue::SoundDialogue()
{
	_toggleStatus = false;
}

void SoundDialogue::Create(UIPopupManager* uim)
{
	__super::Create(uim);
}

void SoundDialogue::Update(float interval)
{
	if (IsKeyPressed(KEY_BACKSPACE)) {
		ui_manager->Close();
	}
}

void SoundDialogue::Draw()
{
	Rectangle r;
	
	if (GuiWindowBox(CenterRectangle(r, 500, 300), "Mute/unmute Sound") == 1)
	{
		ui_manager->Close();
		return;
	}
	GuiCheckBox(CenterRectangle(r, 50, 50), "Mute", &_toggleStatus);
	CenterRectangle(r, 200, 50);
	r.y += 100;
	if (GuiButton(r, "Confirm")) 
	{
		ui_manager->Close();
	}
}

void Demo2eUIManager::Create()
{
	//initialize global UI attributes
	GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

	Menu.Create(this);
	Settings.Create(this);
	Sound.Create(this);
}

UIPopup* Demo2eUIManager::GetPopup(const char* name)
{
	if (!_stricmp(name, "Menu"))
		return &Menu;
	else if (!_stricmp(name, "Settings"))
		return &Settings;
	else if (!_stricmp(name, "Volume"))
		return &Sound;
	return NULL;
}


