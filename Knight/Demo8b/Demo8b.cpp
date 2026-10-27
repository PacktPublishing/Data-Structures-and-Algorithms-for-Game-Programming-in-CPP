#include "Knight.h"
#include "Demo8b.h"

PerspectiveCamera* camera;
SceneActor* Character;
ModelComponent* AnimModelComponent;
float TimeScale = 1.0f;

int AnimationStates[] = { 4, 6 };		//4-Idel, 6-Run
int CurrentAnimIndex = 0;

char strBuf1[100] = "Transition Mode: Immediate";
char strBuf2[100];

int main(int argc, char* argv[])
{
	Demo8b* demo8b = new Demo8b();
	demo8b->Start();
	demo8b->GameLoop();
	delete demo8b;
	return 0;
}

void Demo8b::Start()
{
	__super::Start();

	SetTargetFPS(60);

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{ 0.0f, 6.0f, -15.0f });
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, 30.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -5.0f, 0.0f };
	plane->Scale = Vector3{ 300, 1, 300 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);

	Character = _Scene->CreateSceneObject<SceneActor>("Character");
	Character->Scale = Vector3 { 1.0f, 1.0f, 1.0f };
	Character->Position = Vector3 { 0.0f, 0.0f, 0.0f };
	Character->Rotation.y = 180.0f;
	AnimModelComponent = Character->CreateAndAddComponent<ModelComponent>();
	AnimModelComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	AnimModelComponent->SetAnimation(AnimationStates[CurrentAnimIndex]);   //Set animation 4-idle
	AnimModelComponent->SetAnimationMode(ModelComponent::eAnimMode::Linear_interpolation);
}

void Demo8b::Update(float ElapsedSeconds)
{
	if (IsKeyDown(KEY_LEFT))
	{
		Character->Rotation.y -= 100.0f * ElapsedSeconds;
		if (Character->Rotation.y < -180.0f)
		{
			Character->Rotation.y += 360.0f;
		}
	}
	else if (IsKeyDown(KEY_RIGHT))
	{
		Character->Rotation.y += 60.0f * ElapsedSeconds;
		if (Character->Rotation.y > 180.0f)
		{
			Character->Rotation.y -= 360.0f;
		}
	}

	if (IsKeyPressed(KEY_A))
	{
		CurrentAnimIndex = 1 - CurrentAnimIndex;
		if (AnimModelComponent->GetTransitionMode() == ModelComponent::eAnimTransitionMode::Immediate)
		{
			AnimModelComponent->SetAnimation(AnimationStates[CurrentAnimIndex]);
		}
		else
		{
			AnimModelComponent->TransitionAnimation(AnimationStates[CurrentAnimIndex], 0.5f);
		}
	}

	if (IsKeyPressed(KEY_T))
	{
		auto animTransitionMode = AnimModelComponent->GetTransitionMode();
		if (animTransitionMode == ModelComponent::eAnimTransitionMode::Immediate)
		{
			strcpy_s(strBuf1, 100, "Transition Mode: Linear");
			AnimModelComponent->SetTransitionMode(ModelComponent::eAnimTransitionMode::Linear);
		}
		else if (animTransitionMode == ModelComponent::eAnimTransitionMode::Linear)
		{
			strcpy_s(strBuf1, 100, "Transition Mode: EaseIn/EaseOut");
			AnimModelComponent->SetTransitionMode(ModelComponent::eAnimTransitionMode::EaseInEaseOut);
		}
		else
		{
			strcpy_s(strBuf1, 100, "Transition Mode: Immediate");
			AnimModelComponent->SetTransitionMode(ModelComponent::eAnimTransitionMode::Immediate);
		}
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

void Demo8b::DrawFrame()
{
	__super::DrawFrame();

}


void Demo8b::DrawGUI()
{
	int inputHelpX = 600;
	DrawText("Left and Right arrows: Rotate", inputHelpX, 10, 30, GREEN);
	DrawText("A: Toggle between the Idle and Run animations", inputHelpX, 50, 30, GREEN);
	DrawText("T: Toggle between the Immediate, Linear, and EaseInEaseOut transition modes", inputHelpX, 90, 30, GREEN);
	DrawText("Up and Down arrows: Increase/Descrease time scale", inputHelpX, 130, 30, GREEN);
	
	DrawText("Animation Mode: Linear", 10, 30, 30, GREEN);
	DrawText(strBuf1, 10, 60, 30, GREEN);
	sprintf_s(strBuf2, 100, "Time Scale: %.1f", TimeScale);
	DrawText(strBuf2, 10, 90, 30, GREEN);
}

// Load default resources for the demo
void Demo8b::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);

	//Set the default light data
	_Scene->Lights[0].enabled = true;
	_Scene->Lights[0].color = WHITE;
	_Scene->Lights[0].position = Vector3{ 20.0f, 20.0f, -50.0f };
	_Scene->Lights[0].target = Vector3{ 0.0f, 0.0f, 0.0f };
	_Scene->Lights[0].dirty = true;
}