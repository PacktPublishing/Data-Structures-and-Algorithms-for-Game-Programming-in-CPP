#include "Demo7HMap.h"

//Main entry point for the demo
int main(int argc, char* argv[])
{
	Demo7HMap* KnightDemo7Billboard = new Demo7HMap();

	KnightDemo7Billboard->Start();
	KnightDemo7Billboard->GameLoop();

	delete KnightDemo7Billboard;
	return 0;
}

// Initialize demo with a default scene, camera, and heightmap terrain
void Demo7HMap::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	pMainCamera = _Scene->CreateSceneObject<PerspectiveCamera>("Main Camera");
	pMainCamera->SetFovY(45.0f);
	pMainCamera->SetPosition(Vector3{ 36, 21, 36 });
	pMainCamera->SetLookAtPosition(Vector3{ 0, 0, 0 });
	pMainCamera->CameraMode = CAMERA_ORBITAL;
	pMainCamera->ShowCursor = true;

	pTerrain = _Scene->CreateSceneObject<SceneActor>("Terrain");
	pTerrain->Position = Vector3 { -16.0f, 0.0f, -16.0f };           // Define model position
	pTerrain->Scale = Vector3{ 1,1,1 };
	HMapTerrainModelComponent* heightMap = pTerrain->CreateAndAddComponent<HMapTerrainModelComponent>();
	Vector3 terrainDimension = Vector3{32,8,32};
	bool success = heightMap->CreateFromFile(terrainDimension, Vector2{ 8,8 }, (Config.ResourcesBasePath + "textures/heightmap.png").c_str(), (Config.ResourcesBasePath + "textures/terrain_map.png").c_str());

	UnloadImage(image);             // Unload heightmap image from RAM, already uploaded to VRAM
}

// Release resources and end the game
void Demo7HMap::EndGame()
{
	UnloadTexture(texture);     // Unload texture

	__super::EndGame();
}

void Demo7HMap::DrawFrame()
{
	__super::DrawFrame();
	DrawGrid(20, 1.0f);
}

void Demo7HMap::DrawGUI()
{
	__super::DrawGUI();
	
	DrawText("Use the mouse wheel to zoom camera.", 10, 50, 40, WHITE);
}


void Demo7HMap::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}