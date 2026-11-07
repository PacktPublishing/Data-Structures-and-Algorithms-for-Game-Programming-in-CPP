#include "Knight.h"
#include "Demo8c.h"

PerspectiveCamera* camera;
SceneActor* Character;
ModelComponent* AnimModelComponent;
int FrameRate;
float TimeScale = 1.0f;
char AnimMode[100] = "Animation Mode: Default";

int main(int argc, char* argv[])
{
	Demo8c* demo8c = new Demo8c();
	demo8c->Start();
	demo8c->GameLoop();
	delete demo8c;
	return 0;
}

void Demo8c::Start()
{
	__super::Start();

	FrameRate = 60;
	SetTargetFPS(FrameRate);
	Config.ShowFPS = true;

	camera = _Scene->CreateSceneObject<PerspectiveCamera>("Camera");
	camera->SetPosition(Vector3{0.0f, 15.0f, 30.0f});
	camera->CameraMode = CameraMode::CAMERA_CUSTOM;
	camera->ShowCursor = false;
	camera->SetLookAtPosition(Vector3{ 0.0f, 0.0f, -10.0f });

	SceneActor* plane = _Scene->CreateSceneObject<SceneActor>("Plane");
	plane->Position = Vector3{ 0.0f, -10.0f, 0.0f };
	plane->Scale = Vector3{ 300, 1, 300 };
	PlaneComponent* planeComponent = plane->CreateAndAddComponent<PlaneComponent>();
	planeComponent->SetColor(DARKGREEN);
	SetTargetPosition(Vector3{ 0, 10, 0}, false);
}

void Demo8c::Update(float ElapsedSeconds)
{
	int newFrameRate = FrameRate;
	if (IsKeyPressed(KEY_SPACE))
	{
		InverseKinematics();
	}

	float moveSpeed = 10.0f;

	if (IsKeyDown(KEY_UP))
	{
		SetTargetPosition(Vector3Add(_TargetPosition, Vector3{ 0, ElapsedSeconds * moveSpeed, 0 }));
	}
	if (IsKeyDown(KEY_DOWN))
	{
		SetTargetPosition(Vector3Add(_TargetPosition, Vector3{ 0, -ElapsedSeconds * moveSpeed, 0 }));
	}
	if (IsKeyDown(KEY_LEFT))
	{
		SetTargetPosition(Vector3Add(_TargetPosition, Vector3{ -ElapsedSeconds * moveSpeed, 0, 0 }));
	}
	if (IsKeyDown(KEY_RIGHT))
	{
		SetTargetPosition(Vector3Add(_TargetPosition, Vector3{ ElapsedSeconds * moveSpeed, 0, 0 }));
	}

	if (IsKeyDown(KEY_W))
	{
		SetTargetPosition(Vector3Add(_TargetPosition, Vector3{ 0, 0, -ElapsedSeconds * moveSpeed }));

	}
	if (IsKeyDown(KEY_S))
	{
		SetTargetPosition(Vector3Add(_TargetPosition, Vector3{ 0, 0, ElapsedSeconds * moveSpeed }));
	}

	__super::Update(ElapsedSeconds * TimeScale);
}

void Demo8c::DrawFrame()
{
	__super::DrawFrame();
	DrawRoboticArm();
}


void Demo8c::DrawGUI()
{
	int inputHelpX = 500;
	DrawText("Up, Down, Left, Right: Move the target", inputHelpX, 10, 30, GREEN);
	DrawText("W, S: Move the target forward and backward.", inputHelpX, 50, 30, GREEN);
}


void Demo8c::SetTargetPosition(Vector3 TargetPosition, bool UpdateArm)
{
	_TargetPosition = TargetPosition;

	if (UpdateArm)
	{
		InverseKinematics();
	}
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         

void Demo8c::InverseKinematics()
{
	for (int i = 0; i < IK_MAX_ITERATIONS; ++i)
	{
		Vector3 endEffector = ForwardKinematics();
		Vector3 error = Vector3Subtract(_TargetPosition, endEffector);
		float errorLength = Vector3Length(error);
		if (errorLength <= IK_MIN_ERROR)
		{
			break;
		}
		if (i == 0 || _MinErrorLength > errorLength)
		{
			_MinErrorLength = errorLength;
			_BestJointAngles[0] = _JointAngles[0];
			_BestJointAngles[1] = _JointAngles[1];
		}
		ComputeJacobianMatrix();
		_DeltaAngles[0] = Vector3
		{
			error.x * _JacobianMatrix[0] + error.y * _JacobianMatrix[1] + error.z * _JacobianMatrix[2],
			error.y * _JacobianMatrix[3] + error.y * _JacobianMatrix[4] + error.z * _JacobianMatrix[5],
			error.z * _JacobianMatrix[6] + error.y * _JacobianMatrix[7] + error.z * _JacobianMatrix[8]
		};
		_DeltaAngles[1] = Vector3
		{
			error.x * _JacobianMatrix[9] + error.y * _JacobianMatrix[10] + error.z * _JacobianMatrix[11],
			error.y * _JacobianMatrix[12] + error.y * _JacobianMatrix[13] + error.z * _JacobianMatrix[14],
			error.z * _JacobianMatrix[15] + error.y * _JacobianMatrix[16] + error.z * _JacobianMatrix[17]
		};

		for (int i = 0; i < IK_NUMBER_OF_JOINTS; ++i)
		{
			_JointAngles[i] = Vector3Add(_JointAngles[i], Vector3Scale(_DeltaAngles[i], IK_LEARNING_RATE));
		}
	}
	_JointAngles[0] = _BestJointAngles[0];
	_JointAngles[1] = _BestJointAngles[1];
}

Vector3 Demo8c::ForwardKinematics(Vector3* Angles)
{
	if (Angles == nullptr)
	{
		Angles = _JointAngles;
	}

	Vector3 endEffector { 0 };
	for (int i = 0; i < IK_NUMBER_OF_JOINTS; ++i)
	{
		Matrix matRot = MatrixRotateZYX(Angles[i]);
		Vector3 jointVec = Vector3Transform(_JointLengths[i], matRot);
		endEffector = Vector3Add(endEffector, jointVec);
	}
	return endEffector;
}

void Demo8c::ComputeJacobianMatrix()
{
	Vector3 endEffector = ForwardKinematics();

	for (int i = 0; i < IK_NUMBER_OF_JOINTS; ++i) 
	{
		for (int axis = 0; axis < 3; ++axis)
		{
			Vector3 tempAngles[IK_NUMBER_OF_JOINTS];
			tempAngles[0] = _JointAngles[0];
			tempAngles[1] = _JointAngles[1];
			switch (axis)
			{
			case 0:
				tempAngles[i].x += IK_TOLERANCE;
				break;
			case 1:
				tempAngles[i].y += IK_TOLERANCE;
				break;
			default:
				tempAngles[i].z += IK_TOLERANCE;
			}
			Vector3 pertuebedEndEffector = ForwardKinematics(tempAngles);
			Vector3 differences = Vector3Subtract(pertuebedEndEffector, endEffector);
			int startColumn = i * 9 + axis * 3;
			_JacobianMatrix[startColumn] = differences.x / IK_TOLERANCE;
			_JacobianMatrix[startColumn + 1] = differences.y / IK_TOLERANCE;
			_JacobianMatrix[startColumn + 2] = differences.z / IK_TOLERANCE;
		}
	}
}

void Demo8c::DrawRoboticArm()
{
	DrawSphere(Vector3Zero(), 0.3f, WHITE);
	
	Vector3 points[IK_NUMBER_OF_JOINTS+ 1];
	points[0] = Vector3Zero();
	for (int i = 0; i < IK_NUMBER_OF_JOINTS; ++i)
	{
		Matrix matRot = MatrixRotateZYX(_JointAngles[i]);
		points[i + 1] = Vector3Add(Vector3Transform(_JointLengths[i], matRot), points[i]);
		DrawLine3D(points[i], points[i + 1], i == 0 ? YELLOW : RED);
		DrawSphere(points[i + 1], 0.2f, WHITE);
	}
	DrawSphere(_TargetPosition, 0.5f, RED);
}


void Demo8c::OnConfigureInput()
{
	// The arrows move the IK target in x/y, W/S in z, Space solves.
	Input.AddKeyPad(TouchPad::Left, "Target", KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT);
	Input.AddKeyButton(KEY_SPACE, "SOLVE", TouchButtonAnchor::BottomRight);
	Input.AddKeyButton(KEY_W, "W", TouchButtonAnchor::BottomRight);
	Input.AddKeyButton(KEY_S, "S", TouchButtonAnchor::BottomRight);
}
