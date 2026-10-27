#include "Knight.h"
#include "Demo9a.h"
#include "PlayerFSM.h"

PerspectiveCamera* camera;
SceneActor* Player;
ModelComponent* AnimModelComponent;
PlayerFSM* PlayerCharacterFSM;

int main(int argc, char* argv[])
{
	Demo9a* demo9a = new Demo9a();
	demo9a->Start();
	demo9a->GameLoop();
	delete demo9a;
	return 0;
}

void Demo9a::Start()
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

	Player = _Scene->CreateSceneObject<SceneActor>("PlayerCharacter");
	Player->Position = Vector3{ 0.0f, 0.0f, -10.0f };
	AnimModelComponent = Player->CreateAndAddComponent<ModelComponent>();
	AnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	AnimModelComponent->SetAnimationMode(ModelComponent::Linear_interpolation);
	AnimModelComponent->SetTransitionMode(ModelComponent::Linear);

	PlayerCharacterFSM = new PlayerFSM(Player, AnimModelComponent);
}

void Demo9a::EndGame()
{
	delete PlayerCharacterFSM;
}

void Demo9a::Update(float DeltaTime)
{
	PlayerCharacterFSM->Update(DeltaTime);
	__super::Update(DeltaTime);
}

void Demo9a::DrawGUI()
{
	int inputHelpX = 100;
	DrawText("w,a,d,s: Move", inputHelpX, 10, 30, GREEN);
	DrawText("space: Attack.", inputHelpX, 50, 30, GREEN);
}
