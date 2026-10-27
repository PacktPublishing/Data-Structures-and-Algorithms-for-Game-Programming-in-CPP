// Knight.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "raylib.h"
#include "Knight.h"
#include "rlgl.h"

Knight* Knight::Instance = nullptr;

Knight::Knight()
	: _Scene(nullptr)
	, _Font()
	, _shouldExitGameLoop(false)
{
	Instance = this;
}

Knight::~Knight()
{
}

void Knight::OnCreateDefaultResources()
{
}


void Knight::OnReleaseDefaultResources()
{
}

void Knight::AfterCreateDefaultResources()
{
	//This is called after the default resources are created
	//It perform a check to ensure we have some default resources created

	//Check if user has registered any render passes
	if (_RenderPasses.empty() && Config.EnableDefaultRenderPasses == true)
	{
		TRACELOG(LOG_WARNING, "Knight: No render passes registered, using default ForwardRenderPass");
	
		ForwardRenderPass* pLightPass = nullptr;

		pLightPass = new ForwardRenderPass();
		pLightPass->Create(_Scene);
		_RenderPasses.insert(pLightPass);

		_Scene->_CurrentRenderPass = pLightPass;
	}

	//Make sure we have a default font loaded
	if (_Font.texture.id == 0 || _Font.glyphCount == 0)
	{
		TRACELOG(LOG_WARNING, "Knight: No default font loaded, using default font");
		_Font = LoadFontEx("../../resources/fonts/mecha.png", 40, nullptr, 0);
		if (_Font.texture.id == 0 || _Font.glyphCount == 0)
		{
			TRACELOG(LOG_ERROR, "SYSTEM: Failed to load default font");
			exit(EXIT_FAILURE);
		}
		else {
			TRACELOG(LOG_INFO, "SYSTEM: Default font loaded successfully");
		}
	}
	
	if (Config.EnableDefaultLight == true && _Scene->EnabledLights() == 0)
	{
		TRACELOG(LOG_INFO, "Knight: No light(s) enabled while EnableDefaultLight in configuration file is set to true. Enable a default light");
		//Set the default light data
		_Scene->Lights[0].enabled = true;
		_Scene->Lights[0].color = WHITE;
		_Scene->Lights[0].position = Vector3{ 20.0f, 50.0f, 50.0f };
		_Scene->Lights[0].target = Vector3{ 0.0f, 0.0f, 0.0f };
		_Scene->Lights[0].dirty = true;
	}
}

void Knight::AfterReleaseDefaultResources()
{
	if (_Font.texture.id != 0 && _Font.glyphCount != 0)
	{
		UnloadFont(_Font);
	}

	//release resources used by render passes
	if (!_RenderPasses.empty())
	{
		multiset<SceneRenderPass*, ComparePriorityDescending>::iterator it = _RenderPasses.begin();
		while (it != _RenderPasses.end())
		{
			SceneRenderPass* renderPass = *it;
			renderPass->Release();
			it++;
		}
	}

	//release resources used by off-screen render passes
	if (!_OffScreenPasses.empty())
	{
		multiset<SceneRenderPass*, ComparePriorityDescending>::iterator it = _OffScreenPasses.begin();
		while (it != _OffScreenPasses.end())
		{
			SceneRenderPass* renderPass = *it;
			renderPass->Release();
			it++;
		}
	}
}

void Knight::Start()
{
	OnConfigKnightApp();

	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, APP_TITLE);
	SetTargetFPS(TARGET_FPS);

	_Scene = new Scene();

	OnCreateDefaultResources();

	AfterCreateDefaultResources();
}

void Knight::EndGame()
{
	// When the native window has already received a close request, tearing down
	// GLFW a second time can stall in the graphics driver on Windows. The OS
	// releases the window/context as the process returns from main.
	const bool windowCloseRequested = WindowShouldClose();

	OnReleaseDefaultResources();

	UnloadFont(_Font);

	delete _Scene;
	_Scene = nullptr;

	if (!windowCloseRequested)
	{
		CloseWindow();
	}
}

void Knight::GameLoop()
{
	while (!WindowShouldClose() && (!_shouldExitGameLoop))
	{	
		Update(GetFrameTime());

		DrawOffscreen();

		BeginDrawing();
		ClearBackground(DARKGRAY);
		SceneCamera* cameraActor = _Scene->GetMainCameraActor();
		if (cameraActor)
		{
			BeginMode3D(*cameraActor->GetCamera3D());
			DrawFrame();
			EndMode3D();
		}
		else {
			DrawFrame();
		}

		DrawGUI();

		if (Config.ShowFPS)
		{
			DrawFPS(10, 10);
		}

		EndDrawing();
	}

	EndGame();
}

void Knight::Update(float ElapsedSeconds)
{
	_Scene->Update(ElapsedSeconds);
}

void Knight::DrawFrame()
{
	multiset<SceneRenderPass*, ComparePriorityDescending>::iterator it = _RenderPasses.begin();
	while (it != _RenderPasses.end())
	{
		SceneRenderPass* renderPass = *it;

		renderPass->BeginScene();
		renderPass->Render();
		renderPass->EndScene();
		it++;
	}
}

void Knight::DrawOffscreen()
{
	multiset<SceneRenderPass*, ComparePriorityDescending>::iterator it = _OffScreenPasses.begin();
	while (it != _OffScreenPasses.end())
	{
		SceneRenderPass* renderPass = *it;

		renderPass->BeginScene();
		renderPass->Render();
		renderPass->EndScene();
		it++;
	}
}

void Knight::DrawGUI()
{

}

void Knight::ExitGameLoop()
{
	_shouldExitGameLoop = true;
}

void Knight::DrawFPS(int x, int y)
{
	char text[32];
	sprintf_s(text, "FPS: %i", GetFPS());
	DrawTextEx(_Font, text, Vector2{ (float)x, (float)y }, 40, 3, ORANGE);
}

void Knight::DrawText(const char* text, int x, int y, int size, const Color& color)
{
	DrawTextEx(_Font, text, Vector2{ (float)x, (float)y }, (float)size, 3, color);
}

void Knight::OnConfigKnightApp()
{

}

void Knight::SaveScreenshot(const char* fileName)
{
	// Security check to (partially) avoid malicious code
	if (strchr(fileName, '\'') != NULL) 
	{ 
		printf("SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character"); 
		return; 
	}

	unsigned char* imgData = rlReadScreenPixels(SCREEN_WIDTH, SCREEN_HEIGHT);
	Image image = { imgData, SCREEN_WIDTH, SCREEN_HEIGHT, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

	char path[512] = { 0 };
	strcpy_s(path, TextFormat("%s/%s", "./", GetFileName(fileName)));

	ExportImage(image, path);
	RL_FREE(imgData);

	if (!FileExists(path)) 
		TRACELOG(LOG_WARNING, "Knight: Screenshot could not be saved to [%s]", path);
}
