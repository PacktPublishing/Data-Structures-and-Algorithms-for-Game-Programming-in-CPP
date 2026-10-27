// Bonus Match-3 Game Demo  
#include "BonusMatch3Demo.h"

//main entry point for the application
int main(int argc, char* argv[])
{
    BonusMatch3Demo* KnightMatch3 = new BonusMatch3Demo();

    KnightMatch3->Start();
    KnightMatch3->GameLoop();

    delete KnightMatch3;
    return 0;
}

// This function is called to initialize the game state, including the camera and entities.
void BonusMatch3Demo::Start()
{
    //Initialize Knight Engine with a default scene and camera
    __super::Start();

    Config.ShowFPS = true;
	g.Create();
}

// This function is called to update the game state, including the entities and the camera.
void BonusMatch3Demo::Update(float ElapsedSeconds)
{
	g.Update(ElapsedSeconds);

    //Update rendering settings of all SceneActors and camera position 
    __super::Update(ElapsedSeconds);
}

void BonusMatch3Demo::DrawFrame()
{
	g.Draw();
    __super::DrawFrame();
}

// This function is called when the application is created.
// It is used to load default resources such as fonts.
void BonusMatch3Demo::OnCreateDefaultResources()
{
    __super::OnCreateDefaultResources();
    //Loads a better TrueType font to display text information on the screen
    _Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}

