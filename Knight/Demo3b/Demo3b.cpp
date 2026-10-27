#include "Knight.h"
#include "Demo3b.h"
#include "Random.h"
#include "Selector.h"

PerspectiveCamera* camera;
SceneActor* Castle;
SceneActor* SpawnPointA;
SceneActor* SpawnPointB;
SceneActor* SpawnPointC;
SceneActor* Character1;
SceneActor* Character2;

//Position vectors of the three spwan points
Vector3 SpawnPoints[3] = { {0, 0, 30}, {30, 0, 0}, {0, 0, -30} };

int main(int argc, char* argv[])
{
	Demo3b* demo3b = new Demo3b();
	demo3b->Start();
	demo3b->GameLoop();
	delete demo3b;
	return 0;
}

void Demo3b::Start()
{
	__super::Start();

	Config.ShowFPS = true;

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{80, 60, 0});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0, 10, 0 });

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

	Character1 = _Scene->CreateSceneObject<SceneActor>("Character");
	Character1->Scale = Vector3{ 3.0f, 3.0f, 3.0f };
	Character1->Rotation.y = 90.0f;
	ModelComponent* animModelComponent = Character1->CreateAndAddComponent<ModelComponent>();
	animModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str(),
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, RED);
	animModelComponent->SetAnimation(10);

	Character2 = _Scene->CreateSceneObject<SceneActor>("Character");
	Character2->Scale = Vector3{ 3.0f, 3.0f, 3.0f };
	Character2->Rotation.y = 90.0f;
	animModelComponent = Character2->CreateAndAddComponent<ModelComponent>();
	animModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str(),
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, YELLOW);
	animModelComponent->SetAnimation(10);

	PlaceCharacters();
}

void Demo3b::PlaceCharacters()
{
	Vector3 selectedPositions[2];

	//Select two out of the three spawn points
	if (Selector::ExclusiveRandomSelect(SpawnPoints, 3, selectedPositions, 2))
	{
		//Relocate the two characters at positions of the two selected spawn points
		Character1->Position = selectedPositions[0];
		Character2->Position = selectedPositions[1];
	}
}

void Demo3b::Update(float ElapsedSeconds)
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		PlaceCharacters();
	}

	__super::Update(ElapsedSeconds);
}
