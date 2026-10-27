#include "Demo5Ortho.h"

#include <ctime>

int main(int argc, char* argv[])
{
	Demo5Ortho* KnightDemo5Ortho = new Demo5Ortho();

	KnightDemo5Ortho->Start();
	KnightDemo5Ortho->GameLoop();

	delete KnightDemo5Ortho;
	return 0;
}

Demo5Ortho::Demo5Ortho()
{
	srand(static_cast<unsigned int>(time(nullptr)));
}

void Demo5Ortho::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	OrthCam = _Scene->CreateSceneObject<OrthogonalCamera>("Orthogonal Camera");
	OrthCam->SetUp(Vector3{ 0.0f, 15.0f, 15.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, 20.0f);
}

// Update the camera position based on user input
void Demo5Ortho::DrawFrame()
{
	__super::DrawFrame();

	DrawGrid(20, 1.0f);

	Vector3 pos = OrthCam->GetCamera3D()->target;
	for(int i=-1;i< 2;i++)
	{
		for(int j=-1;j< 2;j++)
		{
			pos.x = OrthCam->GetCamera3D()->target.x + i * 2.0f;
			pos.z = OrthCam->GetCamera3D()->target.z + j * 2.0f;

			DrawCube(pos, 1.0f, 1.0f, 1.0f, PURPLE);
		}
	}
	DrawCubeWires(pos, 1.0f, 1.0f, 1.0f, YELLOW);
}

//Render help text on the screen
void Demo5Ortho::DrawGUI()
{
	__super::DrawGUI();
	DrawText("Rotate camera angle: move mouse.", 10, 100, 40, WHITE);
	DrawText("Move camera: Use arrow keys.", 10, 150, 40, WHITE);
}

// Load default resources for the demo
void Demo5Ortho::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();

	UnloadFont(_Font);
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
