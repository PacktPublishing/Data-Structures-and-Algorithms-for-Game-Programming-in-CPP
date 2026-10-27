#include "Demo9b.h"
#include "Knight.h"
#include "PlayerFSM.h"
#include "EnemyFSM.h"
#include "BehaviourTree.h"

PerspectiveCamera* camera;
SceneActor* Player;
ModelComponent* AnimModelComponent;
PlayerFSM* PlayerCharacterFSM;

SceneActor* Enemy;
ModelComponent* EnemyAnimModelComponent;

EnemyFSM* EnemyCharacterFSM;
BehaviourTree* EnemyBT;

int main(int argc, char* argv[])
{
	Demo9b* demo9b = new Demo9b();
	demo9b->Start();
	demo9b->GameLoop();
	delete demo9b;
	return 0;
}

void Demo9b::Start()
{
	__super::Start();

	SetTargetFPS(30);

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{0.0f, 30.0f, -30.0f});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, 0.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -5.0f, 0.0f };
	plane->Scale = Vector3{ 60, 1, 60 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Player = _Scene->CreateSceneObject<SceneActor>("Character");
	Player->Position = Vector3{ 0.0f, 0.0f, -10.0f };
	AnimModelComponent = Player->CreateAndAddComponent<ModelComponent>();
	AnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	AnimModelComponent->SetAnimationMode(ModelComponent::Linear_interpolation);
	AnimModelComponent->SetTransitionMode(ModelComponent::Linear);
	PlayerCharacterFSM = new PlayerFSM(Player, AnimModelComponent);

	Enemy = _Scene->CreateSceneObject<SceneActor>("Enemy");
	Enemy->Position = Vector3{ 0.0f, 0.0f, 15.0f };
	Enemy->Rotation = Vector3{ 0.0f, 180.0f, 0.0f };
	EnemyAnimModelComponent = Enemy->CreateAndAddComponent<ModelComponent>();
	EnemyAnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str(), 
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, RED);
	EnemyAnimModelComponent->SetAnimationMode(ModelComponent::Linear_interpolation);
	EnemyAnimModelComponent->SetTransitionMode(ModelComponent::Linear);
	EnemyCharacterFSM = new EnemyFSM(Enemy, EnemyAnimModelComponent);
	EnemyBT = new BehaviourTree(Enemy, EnemyCharacterFSM, Player);
}

void Demo9b::EndGame()
{
	delete EnemyBT;
	delete EnemyCharacterFSM;
	delete PlayerCharacterFSM;
}

void Demo9b::Update(float DeltaTime)
{
	PlayerCharacterFSM->Update(DeltaTime);
	EnemyBT->Update(DeltaTime);

	__super::Update(DeltaTime);
}

void Demo9b::DrawGUI()
{
	int inputHelpX = 100;
	DrawText("w,a,d,s: Move", inputHelpX, 10, 30, GREEN);
	DrawText("space: Attack.", inputHelpX, 50, 30, GREEN);
}
