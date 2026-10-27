#include "Knight.h"
#include "Demo3a.h"
#include "Random.h"
#include "Selector.h"

PerspectiveCamera* camera;
SceneActor* Castle;
SceneActor* SpawnPointA;
SceneActor* SpawnPointB;
SceneActor* SpawnPointC;
SceneActor* Character;

//Position vectors of the three spwan points
Vector3 SpawnPoints[3] = { {0, 0, 30}, {30, 0, 0}, {0, 0, -30} };

//Weight values for the spawn points
float SpawnProbabilities[3] = { 0.5f, 0.25f, 0.25f };

int main(int argc, char* argv[])
{
	Demo3a* demo3a = new Demo3a();
	demo3a->Start();
	demo3a->GameLoop();
	delete demo3a;
	return 0;
}

void Demo3a::Start()
{
	__super::Start();

	Config.ShowFPS = true;

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{80, 60, 0});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition( Vector3{ 0, 10, 0 });

	Castle = _Scene->CreateSceneObject<SceneActor>("Castle");
	ModelComponent* modelComponent = Castle->CreateAndAddComponent<ModelComponent>();
	modelComponent->Load3DModel((Config.ResourcesBasePath + "models/obj/castle.obj").c_str(), (Config.ResourcesBasePath + "models/obj/castle_diffuse.png").c_str());

	SpawnPointA = _Scene->CreateSceneObject<SceneActor>("SpawnPoint A");
	SpawnPointA->Position = SpawnPoints[0];
	SpawnPointA->Scale = Vector3{ 0.2f, 0.2f, 0.2f };
	SphereComponent* sphereComponentA = SpawnPointA->CreateAndAddComponent<SphereComponent>();
	sphereComponentA->SetColor(RED);
	sphereComponentA->Radius = 5.0f;

	SpawnPointB = _Scene->CreateSceneObject<SceneActor>("SpawnPoint B");
	SpawnPointB->Position = SpawnPoints[1];
	SpawnPointB->Scale = Vector3{ 0.2f, 0.2f, 0.2f };
	SphereComponent* sphereComponentB = SpawnPointB->CreateAndAddComponent<SphereComponent>();
	sphereComponentB->SetColor(GREEN);
	sphereComponentB->Radius = 5.0f;

	SpawnPointC = _Scene->CreateSceneObject<SceneActor>("SpawnPoint C");
	SpawnPointC->Position = SpawnPoints[2];
	SpawnPointC->Scale = Vector3{ 0.2f, 0.2f, 0.2f };
	SphereComponent* sphereComponentC = SpawnPointC->CreateAndAddComponent<SphereComponent>();
	sphereComponentC->SetColor(BLUE);
	sphereComponentC->Radius = 5.0f;

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0, -5, 0 };
	plane->Scale = Vector3{ 100, 1, 100 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Character = _Scene->CreateSceneObject<SceneActor>("Character");
	Character->Scale = Vector3{ 3.0f, 3.0f, 3.0f };
	Character->Position = Selector::WeightedRandomSelect<Vector3>(SpawnPoints, SpawnProbabilities, 3);
	Character->Rotation.y = 90.0f;
	ModelComponent* animModelComponent = Character->CreateAndAddComponent<ModelComponent>();
	animModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animModelComponent->SetAnimation(10);
}

void Demo3a::Update(float ElapsedSeconds)
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Character->Position = Selector::WeightedRandomSelect<Vector3>(SpawnPoints, SpawnProbabilities, 3);
	}

	__super::Update(ElapsedSeconds);
}
