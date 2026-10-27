#include "Knight.h"
#include "Demo8a.h"

PerspectiveCamera* camera;
SceneActor* Character;
ModelComponent* AnimModelComponent;
int FrameRate;
float TimeScale = 1.0f;
char AnimMode[100] = "Animation Mode: Default";

int main(int argc, char* argv[])
{
	Demo8a* demo8a = new Demo8a();
	demo8a->Start();
	demo8a->GameLoop();
	delete demo8a;
	return 0;
}

void Demo8a::Start()
{
	__super::Start();

	FrameRate = 30;
	SetTargetFPS(FrameRate);

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{0.0f, 20.0f, -30.0f});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, 30.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -5.0f, 0.0f };
	plane->Scale = Vector3{ 300, 1, 300 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Character = _Scene->CreateSceneObject<SceneActor>("Character");
	Character->Scale = Vector3 { 12.0f, 12.0f, 12.0f };
	Character->Position = Vector3 { 0.0f, 0.0f, 0.0f };
	Character->Rotation.y = -90.0f;
	AnimModelComponent = Character->CreateAndAddComponent<ModelComponent>();
	AnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/m3d/cesium_man.m3d").c_str());
	AnimModelComponent->SetAnimation(0);
}

void Demo8a::Update(float ElapsedSeconds)
{
	int newFrameRate = FrameRate;
	if (IsKeyPressed(KEY_ONE))
	{
		FrameRate = 5;
	}
	else if (IsKeyPressed(KEY_THREE))
	{
		FrameRate = 30;
	}
	else if (IsKeyPressed(KEY_SIX))
	{
		FrameRate = 60;
	}
	if (newFrameRate != FrameRate)
	{
		SetTargetFPS(FrameRate);
	}

	if (IsKeyPressed(KEY_A))
	{
		auto animMode = AnimModelComponent->GetAnimationMode();
		ModelComponent::eAnimMode newMode = ModelComponent::eAnimMode::Default;
		switch (animMode)
		{
		case ModelComponent::eAnimMode::Default :
			newMode = ModelComponent::eAnimMode::Linear_interpolation;
			strcpy_s(AnimMode, 50, "Animation Mode : Linear");
			break;
		case ModelComponent::eAnimMode::Linear_interpolation :
			newMode = ModelComponent::eAnimMode::Exponential_interpolation;
			strcpy_s(AnimMode, 50, "Animation Mode : Exponential");
			break;
		default :
			strcpy_s(AnimMode, 50, "Animation Mode : Default");
			break;
		}
		AnimModelComponent->SetAnimationMode(newMode);
	}

	if (IsKeyPressed(KEY_UP) && TimeScale < 2.0f)
	{
		TimeScale += 0.1f;
	}
	else if (IsKeyPressed(KEY_DOWN) && TimeScale > 0.0f)
	{
		TimeScale -= 0.1f;
	}

	__super::Update(ElapsedSeconds * TimeScale);
}

void Demo8a::DrawGUI()
{
	int inputHelpX = 500;
	DrawText("1: Set FPS 5", inputHelpX, 10, 30, GREEN);
	DrawText("3: Set FPS 30.", inputHelpX, 50, 30, GREEN);
	DrawText("6: Set FPS 60.", inputHelpX, 100, 30, GREEN);
	DrawText("A: Toggle animation mode among Default, Linear, and Exponential", inputHelpX, 150, 30, GREEN);
	DrawText("Up and Down arrows: Increase/Descrease time scale", inputHelpX, 200, 30, GREEN);
	
	DrawText(AnimMode, 10, 30, 30, GREEN);

	char strBuf[50];
	sprintf_s(strBuf, 50, "Time Scale: %.1f", TimeScale);
	DrawText(strBuf, 10, 60, 30, GREEN);
}
