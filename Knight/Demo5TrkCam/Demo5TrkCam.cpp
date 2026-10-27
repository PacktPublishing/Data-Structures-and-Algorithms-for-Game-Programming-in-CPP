#include "Demo5TrkCam.h"

//Main application
int main(int argc, char* argv[])
{
	Demo5TrkCam* KnightDemo5TrkCam = new Demo5TrkCam();

	KnightDemo5TrkCam->Start();
	KnightDemo5TrkCam->GameLoop();

	delete KnightDemo5TrkCam;
	return 0;
}

//Create camera and SceneActors
void Demo5TrkCam::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	//initialize a first person camera
	OnRailCamera = _Scene->CreateSceneObject<WaypointsCamera>("Raill Camera");

	//Place player
	Actor = _Scene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f };
	Actor->Position = Vector3{ -10.f,0.15f,0.f };
	Actor->Rotation = Vector3{ 0,90,0 };
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(6);

	OnRailCamera->SetUp(Actor, 45.0f, CAMERA_PERSPECTIVE);

	DisableCursor();
}

//Update new position based on way points 
void Demo5TrkCam::Update(float ElapsedSeconds)
{
	float actorSpeed = 10.0f;

	Vector3 oldPos = Actor->Position;

	// Update cube position along a straight path
	cubePosition.x += cubeDirection.x * ElapsedSeconds * actorSpeed;
	// Reverse direction when it reaches bounds
	if (cubePosition.x > 10.0f || cubePosition.x < -10.0f) {
		cubeDirection.x = -cubeDirection.x;
	}
	Actor->Position = cubePosition;

	Vector3 vel = Vector3Subtract(Actor->Position, oldPos);
	float angle = atan2f(vel.z, vel.x) * RAD2DEG; // Calculate angle in degrees

	angle += 90;
	if (angle > 360) angle -= 360; // Normalize angle to [0, 360)
	Actor->Rotation = Vector3{ 0, angle, 0 };

	__super::Update(ElapsedSeconds);

}

//Render the scene 
void Demo5TrkCam::DrawFrame()
{
	OnRailCamera->Draw(); //render waypoint as cube
	DrawGrid(50, 3.0f);
	__super::DrawFrame();
}

//Display current status (on waypoint, or moving to next waypoint, etc.)
void Demo5TrkCam::DrawGUI()
{
	__super::DrawGUI();

	if (OnRailCamera->waypointWaitTimer > 0)
		DrawText(TextFormat("Stop at waypoint #%d, wait time left=%1.2f", OnRailCamera->currentWaypoint, OnRailCamera->waypointWaitTimer), 10, 50, 40, WHITE);
	else
		DrawText(TextFormat("Moving from waypoint %d to %d", OnRailCamera->currentWaypoint, (OnRailCamera->currentWaypoint + 1 >= OnRailCamera->waypoints.size())?0: OnRailCamera->currentWaypoint+1), 10, 50, 40, WHITE);
}

// Load default resources for the demo
void Demo5TrkCam::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}