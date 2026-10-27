#include "Knight.h"
#include "Demo9c.h"

PerspectiveCamera* camera;
SceneActor* Character;
ModelComponent* AnimModelComponent;
vector<Vector3> Path;

Vector3 StartPoint = Vector3{ -35.0f, 0.0f, -35.0f };
Vector3 GoalPoint = Vector3{35.0f, 0.0f, 35.0f};
int NextWaypoint = 0;

int main(int argc, char* argv[])
{
	Demo9c* demo9c = new Demo9c();
	demo9c->Start();
	demo9c->GameLoop();
	delete demo9c;
	return 0;
}

void Demo9c::Start()
{
	__super::Start();

	SetTargetFPS(30);

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{0.0f, 60.0f, 60.0f});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, 10.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -5.0f, 0.0f };
	plane->Scale = Vector3{ 100, 1, 100 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Character = _Scene->CreateSceneObject<SceneActor>("Character");
	Character->Position = Vector3 { -35.0f, 0.0f, -35.0f };
	Character->Rotation.y = 90.0f;
	AnimModelComponent = Character->CreateAndAddComponent<ModelComponent>();
	AnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	AnimModelComponent->SetAnimation(6);
}

void Demo9c::Update(float DeltaTime)
{
	if (IsKeyPressed(KEY_SPACE))
	{
		Maze::GenerateMaze();
		Path = _pathFinder.FindPath(StartPoint, GoalPoint);
		Character->Position = StartPoint;
		NextWaypoint = 1;
	}
	else if (!Path.empty())
	{
		float walkSpeed = 5.0f;
		Vector3 dir = Vector3Subtract(Path[NextWaypoint], Path[NextWaypoint - 1]);
		Vector3 vel = Vector3Scale(Vector3Normalize(dir), walkSpeed * DeltaTime);
		if (Vector3Distance(Character->Position, Path[NextWaypoint]) <= Vector3Length(vel))
		{
			Character->Position = Path[NextWaypoint];
			Character->Rotation.y = RAD2DEG * atan2f(dir.x, dir.z);
			NextWaypoint++;
			if (NextWaypoint >= Path.size())
			{
				Path.clear();
			}
		}
		else
		{
			Character->Position = Vector3Add(Character->Position, vel);
			Character->Rotation.y = RAD2DEG * atan2f(dir.x, dir.z);
		}
	}

	__super::Update(DeltaTime);
}

void Demo9c::DrawFrame()
{
	__super::DrawFrame();

	Vector2 start, end;

	start.x = -(float)Maze::CellSize * Maze::Cols / 2;
	start.y = (float)Maze::CellSize * Maze::Rows / 2;
	end.x = (float)Maze::CellSize * Maze::Cols / 2;
	end.y = -(float)Maze::CellSize * Maze::Rows / 2;

	//Draw the grid frame
	for (int row = 0; row < Maze::Rows + 1; ++row)
	{
		float z = (float)(row - 4) * Maze::CellSize;
		DrawLine3D(Vector3{ start.x, 0, z }, Vector3{ end.x, 0, z}, ORANGE);
	}
	for (int col = 0; col < Maze::Cols + 1; ++col)
	{
		float x = (float)(col - 4) * Maze::CellSize;
		DrawLine3D(Vector3{ x, 0, start.y }, Vector3{ x, 0, end.y }, ORANGE);
	}

	//Draw blockers
	for (int i = 0; i < GridsSize; ++i)
	{
		if (Maze::MazeGrids[i] > 0)
		{
			int row, col;
			if (Maze::GetMazeCellRowCol(i, row, col))
			{
				float x = (float)(col - 4) * Maze::CellSize + Maze::CellSize / 2;
				float z = (float)(row - 4) * Maze::CellSize + Maze::CellSize / 2;
				DrawCubeV(Vector3{x, 0.0f, z}, Vector3{7, 7, 7}, BLUE);
			}
		}
	}

	//Draw Goal
	DrawSphere(GoalPoint, 1.0f, RED);

	//Draw Path
	for(int i = 0; i < (int)(Path.size() - 1); ++i)
	{
		//DrawLine3D(Path[i], Path[i + 1], RED);
		DrawCylinderEx(Path[i], Path[i + 1], 0.3f, 0.3f, 8, RED);
	}

}

void Demo9c::DrawGUI()
{
	DrawText("Space: Start ... ", 10, 10, 30, GREEN);
}

