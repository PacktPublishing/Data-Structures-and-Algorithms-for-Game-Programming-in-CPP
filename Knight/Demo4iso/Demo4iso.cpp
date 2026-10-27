#include "Knight.h"
#include "Demo4iso.h"

//Main function for the application
int main(int argc, char* argv[])
{
	Demo4iso* KnightDemo4iso = new Demo4iso();

	KnightDemo4iso->Start();
	KnightDemo4iso->GameLoop();

	delete KnightDemo4iso;
	return 0;
}

//Initialize tiles in the map with random values
Demo4iso::Demo4iso()
{
	for (int y = 0; y < 10; ++y) {
		for (int x = 0; x < 10; ++x) {
			map[x][y] = rand() % 96; 
		}
	}
}

//Create map
void Demo4iso::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Image image = LoadImage((Config.ResourcesBasePath + "textures/seasons_tiles.png").c_str());     // Loaded in CPU memory (RAM)
	_mapTiles = LoadTextureFromImage(image);          // Image converted to texture, GPU memory (VRAM)
	UnloadImage(image);   // Once image has been converted to texture and uploaded to VRAM, it can be unloaded from RAM
}

//Unload resources used by the game
void Demo4iso::EndGame()
{
	UnloadTexture(_mapTiles);
	__super::EndGame();
}

//Render the isometric map on the screen
void Demo4iso::DrawGUI()
{
	for (int y = 0; y < 10; ++y) {
		for (int x = 0; x < 10; ++x) {
			// Convert 2D grid coordinates to isometric screen coordinates
			int screenX = (x - y) * 128 / 2+ SCREEN_WIDTH/2;
			int screenY = (x + y) * 64 / 2+ SCREEN_HEIGHT/4;

			Rectangle r = {(float)(map[x][y] % 8)*128,(float)(map[x][y]/12)*64, 128.0f, 64.0f};
			Vector2 v2;
			v2.x = (float)screenX; v2.y = (float)screenY;
			DrawTextureRec(_mapTiles, r, v2, WHITE);
		}
	}

	DrawText("Rendering isometric map", 10, 50, 32, WHITE);
}

//Create default resources for the game
void Demo4iso::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
