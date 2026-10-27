#include "Knight.h"
#include "Demo10.h"
#include "PlayerFSM.h"
#include "TurretController.h"

PerspectiveCamera* camera;
SceneActor* Player;
SceneActor* Cannon;
TurretController* Turret;
ModelComponent* AnimModelComponent;
PlayerFSM* PlayerCharacterFSM;

int main(int argc, char* argv[])
{
	Demo10* demo10 = new Demo10();
	demo10->Start();
	demo10->GameLoop();
	delete demo10;
	return 0;
}

void Demo10::Start()
{
	__super::Start();

	SetTargetFPS(30);

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{ 0.0f, 30.0f, -30.0f });
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, 0.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -5.0f, 0.0f };
	plane->Scale = Vector3{ 60, 1, 60 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Player = _Scene->CreateSceneObject<SceneActor>("PlayerCharacter");
	Player->Position = Vector3{ 0.0f, 0.0f, -10.0f };
	AnimModelComponent = Player->CreateAndAddComponent<ModelComponent>();
	AnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	AnimModelComponent->SetAnimationMode(ModelComponent::Linear_interpolation);
	AnimModelComponent->SetTransitionMode(ModelComponent::Linear);
	PlayerCharacterFSM = new PlayerFSM(Player, AnimModelComponent);

	SceneActor* turret = _Scene->CreateSceneObject<SceneActor>("Turret");
	turret->Position = Vector3{ 0.0f, 0.0f, 8.0f };
	turret->Scale = Vector3{ 2, 2, 2 };
	CylinderComponent* turretCylinder = turret->CreateAndAddComponent<CylinderComponent>();
	turretCylinder->SetColor(DARKBROWN);

	Cannon = _Scene->CreateSceneObject<SceneActor>("Cannon");
	Cannon->Position = Vector3{ 0.0f, 1.0f, 8.0f };
	Cannon->Rotation = Vector3{ -90.0f, 0.0f, 0.0f };
	Cannon->Scale = Vector3{ 0.5f, 3.5f, 0.5f };
	CylinderComponent* cannonCylinder = Cannon->CreateAndAddComponent<CylinderComponent>();
	cannonCylinder->SetColor(WHITE);
	Turret = new TurretController(_Scene, Cannon);
}	

void Demo10::EndGame()
{
	delete Turret;
	delete PlayerCharacterFSM;
}

void Demo10::Update(float DeltaTime)
{
	if (Turret != nullptr && !Turret->Trained && IsKeyPressed(KEY_F1))
	{
		Turret->Train(1, 10000, 1);
	}
	if (Turret != nullptr && !Turret->Trained && IsKeyPressed(KEY_F2))
	{
		Turret->Train(2, 10000, 3);
	}

	PlayerCharacterFSM->Update(DeltaTime);
	Turret->Update(DeltaTime);
	__super::Update(DeltaTime);
}

void Demo10::DrawGUI()
{
	int inputHelpX = 100;
	DrawText("w,a,d,s: Move", inputHelpX, 10, 30, GREEN);
	if (Turret->Trained)
	{
		DrawText("Turret Controller is trained.", inputHelpX, 50, 30, GREEN);
	}
	else
	{
		DrawText("F1: Train the model with shadow learning", inputHelpX, 50, 30, RED);
		DrawText("F2: Train the model with deep learning", inputHelpX, 70, 30, RED);
	}
}
