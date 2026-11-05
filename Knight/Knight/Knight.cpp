// Knight.cpp : Defines the functions for the static library.
//

#include "pch.h"
#include "raylib.h"
#include "Knight.h"
#include "rlgl.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

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

//This is called after the default resources are created
//It perform a check to ensure we have some default resources created
void Knight::AfterCreateDefaultResources()
{
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
		_Font = LoadFontEx((Knight::Instance->Config.ResourcesBasePath + "fonts/mecha.png").c_str(), 40, nullptr, 0);
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
	//Allow user to configure the Knight application before it starts
	OnConfigKnightApp();

	//Validate the configuration settings and set default values if necessary
	ApplyAppConfig();

	InitWindow(Config.WindowWidth, Config.WindowHeight, Config.WindowTitle.c_str());
	SetTargetFPS(TARGET_FPS);
	OnConfigureInput();

#if defined(PLATFORM_ANDROID)
	// Give legacy samples a usable primary action until they provide a
	// sample-specific command layout. Explicit layouts always win.
	if (Input.TouchButtonCount() == 0)
	{
		InputAction action = Input.RegisterAction("Primary Action");
		Input.BindKey(action, KEY_SPACE);
		Input.AddTouchButton(action, "ACTION", TouchButtonAnchor::TopRight, 0);
	}
#endif

	_Scene = new Scene();

	//Allow user to create default resources, such as font, shaders, textures, etc.
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
#if defined(PLATFORM_WEB)
	emscripten_set_main_loop_arg([](void* userData)
	{
		Knight* app = static_cast<Knight*>(userData);
		if (app->_shouldExitGameLoop)
		{
			emscripten_cancel_main_loop();
			app->EndGame();
			return;
		}

		app->Input.BeginFrame();
		app->Update(GetFrameTime());
		app->DrawOffscreen();

		if (app->Input.IsKeyPressed(KEY_ENTER))
		{
			TakeScreenshot("screenshot.png");
		}
		if (app->Input.IsKeyPressed(KEY_F3))
		{
			app->Config.ShowInputDebug = !app->Config.ShowInputDebug;
		}

		BeginDrawing();
		ClearBackground(DARKGRAY);
		SceneCamera* cameraActor = app->_Scene->GetMainCameraActor();
		if (cameraActor)
		{
			BeginMode3D(*cameraActor->GetCamera3D());
			app->DrawFrame();
			EndMode3D();
		}
		else
		{
			app->DrawFrame();
		}

		app->DrawGUI();
		app->Input.DrawOverlay();
		if (app->Config.ShowInputDebug)
		{
			app->Input.DrawDebugOverlay();
		}
		if (app->Config.ShowFPS)
		{
			app->DrawFPS(10, 10);
		}
		EndDrawing();
	}, this, 0, true);
#else
	while (!WindowShouldClose() && (!_shouldExitGameLoop))
	{	
		Input.BeginFrame();
		Update(GetFrameTime());

#if defined(PLATFORM_ANDROID)
		// raylib maps Android's Back button to KEY_BACK and consumes the OS
		// event, so the app decides what Back means. Escape from a hardware
		// keyboard is treated the same way.
		if (Input.Pressed(Input.BackAction()) || IsKeyPressed(KEY_ESCAPE))
		{
			OnBackRequested();
			if (_shouldExitGameLoop)
				continue;
		}
#endif

		DrawOffscreen();

		if (IsKeyPressed(KEY_ENTER))
		{
			TakeScreenshot("screenshot.png");
		}
		if (IsKeyPressed(KEY_F3))
		{
			Config.ShowInputDebug = !Config.ShowInputDebug;
		}

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
		Input.DrawOverlay();
		if (Config.ShowInputDebug)
		{
			Input.DrawDebugOverlay();
		}
		DrawBackHint();

		if (Config.ShowFPS)
		{
			DrawFPS(10, 10);
		}

		EndDrawing();
	}

	EndGame();
#endif
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

// Default Back button behavior on Android: the first press shows a hint and a
// second press within BackExitSeconds leaves the app, so a stray tap on Back
// does not end the session. Override to pause or return to a menu instead.
void Knight::OnBackRequested()
{
	const double now = GetTime();
	if (_backRequestedTime >= 0.0 && now - _backRequestedTime <= BackExitSeconds)
	{
		ExitGameLoop();
		return;
	}
	_backRequestedTime = now;
}

void Knight::DrawBackHint()
{
	if (_backRequestedTime < 0.0 || GetTime() - _backRequestedTime > BackExitSeconds)
		return;

	const char* text = "Press Back again to exit";
	const float size = GetScreenHeight() * 0.045f;
	const Vector2 extent = MeasureTextEx(_Font, text, size, 3);
	const float x = (GetScreenWidth() - extent.x) * 0.5f;
	const float y = GetScreenHeight() * 0.8f;
	DrawRectangleRounded(Rectangle{ x - size * 0.6f, y - size * 0.3f,
		extent.x + size * 1.2f, extent.y + size * 0.6f }, 0.5f, 8, Color{ 0, 0, 0, 170 });
	DrawTextEx(_Font, text, Vector2{ x, y }, size, 3, WHITE);
}

void Knight::OnConfigKnightApp()
{
	Config.WindowWidth = DEFAULT_SCREEN_WIDTH;
	Config.WindowHeight = DEFAULT_SCREEN_HEIGHT;
	Config.WindowTitle = DEFAULT_APP_TITLE;
	Config.ResourcesBasePath = RESOURCE_ROOT_PATH;
}

void Knight::OnConfigureInput()
{
}

bool Knight::IsMouseButtonDown(int button) const
{
	if (button == MOUSE_BUTTON_LEFT)
		return Input.PrimaryPointer().down;
	if (button == MOUSE_BUTTON_RIGHT)
		return Input.SecondaryPointer().down;
	return ::IsMouseButtonDown(button);
}

bool Knight::IsMouseButtonPressed(int button) const
{
	if (button == MOUSE_BUTTON_LEFT)
		return Input.PrimaryPointer().pressed;
	if (button == MOUSE_BUTTON_RIGHT)
		return Input.SecondaryPointer().pressed;
	return ::IsMouseButtonPressed(button);
}

bool Knight::IsMouseButtonReleased(int button) const
{
	if (button == MOUSE_BUTTON_LEFT)
		return Input.PrimaryPointer().released;
	if (button == MOUSE_BUTTON_RIGHT)
		return Input.SecondaryPointer().released;
	return ::IsMouseButtonReleased(button);
}

void Knight::ApplyAppConfig()
{
	if (Config.WindowWidth <= 0)
		Config.WindowWidth = DEFAULT_SCREEN_WIDTH;
	if (Config.WindowHeight <= 0)
		Config.WindowHeight = DEFAULT_SCREEN_HEIGHT;
	if (Config.WindowTitle.empty())
		Config.WindowTitle = DEFAULT_APP_TITLE;
	if (Config.ResourcesBasePath.empty())
		Config.ResourcesBasePath = RESOURCE_ROOT_PATH;
}

void Knight::SaveScreenshot(const char* fileName)
{
	// Security check to (partially) avoid malicious code
	if (strchr(fileName, '\'') != NULL) 
	{ 
		printf("SYSTEM: Provided fileName could be potentially malicious, avoid [\'] character"); 
		return; 
	}

	unsigned char* imgData = rlReadScreenPixels(Config.WindowWidth, Config.WindowHeight);
	Image image = { imgData, Config.WindowWidth, Config.WindowHeight, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };

	char path[512] = { 0 };
	strcpy_s(path, TextFormat("%s/%s", "./", GetFileName(fileName)));

	ExportImage(image, path);
	RL_FREE(imgData);

	if (!FileExists(path)) 
		TRACELOG(LOG_WARNING, "Knight: Screenshot could not be saved to [%s]", path);
}
