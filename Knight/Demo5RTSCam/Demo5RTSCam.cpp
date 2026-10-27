#include "Demo5RTSCam.h"

#include <cmath>
#include <ctime>

//Main entry point for the demo application
int main(int argc, char* argv[])
{
	Demo5RTSCam* KnightDemo5RTSCam = new Demo5RTSCam();

	KnightDemo5RTSCam->Start();
	KnightDemo5RTSCam->GameLoop();

	delete KnightDemo5RTSCam;
	return 0;
}

void Demo5RTSCam::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	srand(static_cast<unsigned int>(time(nullptr)));

	RTSCamera = _Scene->CreateSceneObject<TopDownCamera>("Chase Camera");

	for (int i = 0; i < 10; i++) {
		UnitEntity u = { "", Vector3 { (float)(rand() % 20 - 10), 0.5f, (float)(rand() % 20 - 10)},false };
		sprintf_s(u.Name, sizeof(u.Name), "Team-%d", i);
		units.push_back(u);
	}

	RTSCamera->SetUp(Vector3{ 0.0f, 15.0f, 15.0f }, Vector3{ 0.0f, 0.0f, 0.0f }, 45.0f, CAMERA_PERSPECTIVE);
}

void Demo5RTSCam::Update(float ElapsedSeconds)
{
	// Mouse picking: check if any unit is clicked
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		Ray ray = GetScreenToWorldRay(GetMousePosition(), *RTSCamera->GetCamera3D());

		for (auto& unit : units) {
			// Check if the ray hits the cube
			BoundingBox box = {
				Vector3 { unit.position.x - 0.5f, unit.position.y - 0.5f, unit.position.z - 0.5f},
				Vector3 { unit.position.x + 0.5f, unit.position.y + 0.5f, unit.position.z + 0.5f}
			};
		
			// Check collision between ray and cube's bounding 
			RayCollision rc = GetRayCollisionBox(ray, box);

			if (rc.hit) {
				unit.selected = !unit.selected;  // Toggle unit selection
			}
		}
	}

	__super::Update(ElapsedSeconds);
}

//Render all units as cubes in the scene
void Demo5RTSCam::DrawFrame()
{
	__super::DrawFrame();

	// Draw grid for better spatial reference
	DrawGrid(20, 1.0f);

	for (const auto& unit : units) {
		Color color = unit.selected ? GREEN : RED;
		DrawCube(unit.position, 1.0f, 1.0f, 1.0f, color);
		DrawCubeWires(unit.position, 1.0f, 1.0f, 1.0f, DARKGRAY);
	}
}

//Render help text and unit names of all units (represened by cubes)
void Demo5RTSCam::DrawGUI()
{
	__super::DrawGUI();

	for (auto& unit : units) {		
		BoundingRect rect = Get2DBoundingRectOfCube(unit.position, 1.0f, *RTSCamera->GetCamera3D());
		unit.LabelPos.x = rect.min.x;
		unit.LabelPos.y = rect.min.y-20;  //on top of the bounding rect
		DrawText(unit.Name, (int)unit.LabelPos.x, (int)unit.LabelPos.y, 30, YELLOW);

	}

	DrawText("Use mouse to select/unselect cubes", 10, 50, 40, YELLOW);
	DrawText("Arrow keys to pan, mouse wheel to zoom", 10, 100, 40, YELLOW);

}

// Load default resources for the demo
void Demo5RTSCam::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}