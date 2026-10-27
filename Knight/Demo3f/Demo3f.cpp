#include "Knight.h"
#include "Demo3f.h"

PerspectiveCamera* camera;
SceneActor* Character;

int main(int argc, char* argv[])
{
	Demo3f* demo3f = new Demo3f();
	demo3f->Start();
	demo3f->GameLoop();
	delete demo3f;
	return 0;
}

void Demo3f::Start()
{
	__super::Start();

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{0.0f, 50.0f, -120.0f});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, 0.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -5.0f, 50.0f };
	plane->Scale = Vector3{ 300, 1, 300 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Character = _Scene->CreateSceneObject<SceneActor>("Character");
	Character->Scale = Vector3 { 3.0f, 3.0f, 3.0f };
	Character->Position = Vector3 { 0.0f, 0.0f, -60.0f };
	Character->Rotation.y = 0.0f;
	ModelComponent* animModelComponent = Character->CreateAndAddComponent<ModelComponent>();
	animModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animModelComponent->SetAnimation(2);
}

void Demo3f::Update(float ElapsedSeconds)
{
	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
	{
		Bullet* bullet = _bulletPool.GetObject();
		Vector3 startPos = Character->Position;
		startPos.y = 15.0f;
		bullet->SetPosition(startPos, Vector3{ 0.0f, 0.0f, 50.0f });
	}

	__super::Update(ElapsedSeconds);
	_bulletPool.Update(ElapsedSeconds);
}

void Demo3f::DrawFrame()
{
	__super::DrawFrame();
	_bulletPool.Draw();
}


void Demo3f::DrawGUI()
{
	DrawText("Left Click: Fire bullets.", 100, 50, 30, GREEN);
}
