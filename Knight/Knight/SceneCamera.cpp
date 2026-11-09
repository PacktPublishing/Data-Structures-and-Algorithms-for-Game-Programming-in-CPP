#include "SceneCamera.h"
#include "Knight.h"
#include "Scene.h"
#include "SceneActor.h"

#include <algorithm> // For std::lower_bound

#include "rlgl.h"
#include "rcamera.h"

namespace
{
constexpr float CameraMoveSpeed = 5.4f;
constexpr float CameraRotationSpeed = 0.03f;
constexpr float CameraMouseSensitivity = 0.003f;
constexpr float CameraOrbitalSpeed = 0.5f;

// raylib's UpdateCamera() reads raw mouse state. On Android raylib also maps
// touches to that state, so a thumb captured by an on-screen control can move
// the camera. Keep raylib's camera behavior, but source pointer, keys and zoom
// from Knight's InputSystem so captured control touches are excluded.
void UpdateCameraFromKnightInput(Camera3D* camera, CameraMode mode, float elapsedSeconds)
{
	const InputSystem& input = KnightInput();
	const Vector2 pointerDelta = input.SecondaryPointer().delta;
	const bool moveInWorldPlane = mode == CAMERA_FIRST_PERSON || mode == CAMERA_THIRD_PERSON;
	const bool rotateAroundTarget = mode == CAMERA_THIRD_PERSON || mode == CAMERA_ORBITAL;
	const bool lockView = mode != CAMERA_CUSTOM;
	const float moveSpeed = CameraMoveSpeed * elapsedSeconds;
	const float rotationSpeed = CameraRotationSpeed * elapsedSeconds;

	if (mode == CAMERA_CUSTOM)
		return;

	if (mode == CAMERA_ORBITAL)
	{
		CameraYaw(camera, CameraOrbitalSpeed * elapsedSeconds, true);
	}
	else
	{
		if (input.IsKeyDown(KEY_DOWN)) CameraPitch(camera, -rotationSpeed, lockView, rotateAroundTarget, false);
		if (input.IsKeyDown(KEY_UP)) CameraPitch(camera, rotationSpeed, lockView, rotateAroundTarget, false);
		if (input.IsKeyDown(KEY_RIGHT)) CameraYaw(camera, -rotationSpeed, rotateAroundTarget);
		if (input.IsKeyDown(KEY_LEFT)) CameraYaw(camera, rotationSpeed, rotateAroundTarget);
		if (input.IsKeyDown(KEY_Q)) CameraRoll(camera, -rotationSpeed);
		if (input.IsKeyDown(KEY_E)) CameraRoll(camera, rotationSpeed);

		CameraYaw(camera, -pointerDelta.x * CameraMouseSensitivity, rotateAroundTarget);
		CameraPitch(camera, -pointerDelta.y * CameraMouseSensitivity,
			lockView, rotateAroundTarget, false);

		if (input.IsKeyDown(KEY_W)) CameraMoveForward(camera, moveSpeed, moveInWorldPlane);
		if (input.IsKeyDown(KEY_A)) CameraMoveRight(camera, -moveSpeed, moveInWorldPlane);
		if (input.IsKeyDown(KEY_S)) CameraMoveForward(camera, -moveSpeed, moveInWorldPlane);
		if (input.IsKeyDown(KEY_D)) CameraMoveRight(camera, moveSpeed, moveInWorldPlane);

		// Hardware gamepads remain independent of touch ownership.
		if (IsGamepadAvailable(0))
		{
			CameraYaw(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X) * 2.0f) *
				CameraMouseSensitivity, rotateAroundTarget);
			CameraPitch(camera, -(GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y) * 2.0f) *
				CameraMouseSensitivity, lockView, rotateAroundTarget, false);

			if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) <= -0.25f) CameraMoveForward(camera, moveSpeed, moveInWorldPlane);
			if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) <= -0.25f) CameraMoveRight(camera, -moveSpeed, moveInWorldPlane);
			if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) >= 0.25f) CameraMoveForward(camera, -moveSpeed, moveInWorldPlane);
			if (GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) >= 0.25f) CameraMoveRight(camera, moveSpeed, moveInWorldPlane);
		}

		if (mode == CAMERA_FREE)
		{
			if (input.IsKeyDown(KEY_SPACE)) CameraMoveUp(camera, moveSpeed);
			if (input.IsKeyDown(KEY_LEFT_CONTROL)) CameraMoveUp(camera, -moveSpeed);
		}
	}

	if (mode == CAMERA_THIRD_PERSON || mode == CAMERA_ORBITAL || mode == CAMERA_FREE)
	{
		CameraMoveToTarget(camera, -input.ZoomDelta());
		if (input.IsKeyPressed(KEY_KP_SUBTRACT)) CameraMoveToTarget(camera, 2.0f);
		if (input.IsKeyPressed(KEY_KP_ADD)) CameraMoveToTarget(camera, -2.0f);
	}
}
}

SceneCamera::SceneCamera(Scene* Scene, const char* Name, bool IsMainCamera) : 
	SceneObject(Scene, Name)
	, _Camera({ 0 })
	, CameraMode(CAMERA_CUSTOM)
	, ShowCursor(true)
{
	_Camera.position = Vector3{ 0,1,-10.0f };
	_Camera.target = Vector3{ 0,0,0 };
	_Camera.fovy = 45.0;
	_Camera.up = Vector3{ 0,1,0 };
	if (IsMainCamera)
		_Scene->AssignMainCamera(this);
}

SceneCamera::~SceneCamera()
{
}

bool SceneCamera::Update(float ElapsedSeconds)
{
	if (!IsActive)
	{
		return false;
	}

	if (CameraMode != CAMERA_CUSTOM)
		UpdateCameraFromKnightInput(&_Camera, CameraMode, ElapsedSeconds);

	__super::Update(ElapsedSeconds); // Call base class update

	return true;
}

Camera3D* SceneCamera::GetCamera3D()
{
	return &_Camera;
}

// Extracts the 6 planes of the view frustum. Normals point inwards.
void SceneCamera::ExtractFrustumPlanes(FrustumPlane frustumPlanes[6])
{
	Camera cam = *GetCamera3D(); // Get the camera from the SceneCamera component

	Matrix matView = GetCameraMatrix(cam);
	Matrix matProj = rlGetMatrixProjection(); // Get current projection matrix from rlgl
	Matrix matViewProj = MatrixMultiply(matView, matProj);

	// Right plane
	frustumPlanes[0].normal.x = matViewProj.m3 - matViewProj.m0;
	frustumPlanes[0].normal.y = matViewProj.m7 - matViewProj.m4;
	frustumPlanes[0].normal.z = matViewProj.m11 - matViewProj.m8;
	frustumPlanes[0].d = matViewProj.m15 - matViewProj.m12;

	// Left plane
	frustumPlanes[1].normal.x = matViewProj.m3 + matViewProj.m0;
	frustumPlanes[1].normal.y = matViewProj.m7 + matViewProj.m4;
	frustumPlanes[1].normal.z = matViewProj.m11 + matViewProj.m8;
	frustumPlanes[1].d = matViewProj.m15 + matViewProj.m12;

	// Bottom plane
	frustumPlanes[2].normal.x = matViewProj.m3 + matViewProj.m1;
	frustumPlanes[2].normal.y = matViewProj.m7 + matViewProj.m5;
	frustumPlanes[2].normal.z = matViewProj.m11 + matViewProj.m9;
	frustumPlanes[2].d = matViewProj.m15 + matViewProj.m13;

	// Top plane
	frustumPlanes[3].normal.x = matViewProj.m3 - matViewProj.m1;
	frustumPlanes[3].normal.y = matViewProj.m7 - matViewProj.m5;
	frustumPlanes[3].normal.z = matViewProj.m11 - matViewProj.m9;
	frustumPlanes[3].d = matViewProj.m15 - matViewProj.m13;

	// Far plane
	frustumPlanes[4].normal.x = matViewProj.m3 - matViewProj.m2;
	frustumPlanes[4].normal.y = matViewProj.m7 - matViewProj.m6;
	frustumPlanes[4].normal.z = matViewProj.m11 - matViewProj.m10;
	frustumPlanes[4].d = matViewProj.m15 - matViewProj.m14;

	// Near plane
	frustumPlanes[5].normal.x = matViewProj.m3 + matViewProj.m2;
	frustumPlanes[5].normal.y = matViewProj.m7 + matViewProj.m6;
	frustumPlanes[5].normal.z = matViewProj.m11 + matViewProj.m10;
	frustumPlanes[5].d = matViewProj.m15 + matViewProj.m14;

	// Normalize the planes
	for (int i = 0; i < 6; i++) {
		float length = Vector3Length(frustumPlanes[i].normal);
		if (length == 0.0f) length = 1.0f; // Avoid division by zero, though unlikely for valid VP matrix
		float invLength = 1.0f / length;
		frustumPlanes[i].normal = Vector3Scale(frustumPlanes[i].normal, invLength);
		frustumPlanes[i].d *= invLength;
	}
}

// Check if a bounding box is inside or intersects the frustum
// Uses the "sphere vs plane" test for each plane, where the sphere encloses the AABB corner
bool SceneCamera::IsBoundingBoxInFrustum(BoundingBox box, const FrustumPlane frustumPlanes[6])
{
	Vector3 center = { (box.min.x + box.max.x) * 0.5f, (box.min.y + box.max.y) * 0.5f, (box.min.z + box.max.z) * 0.5f };
	Vector3 halfSize = { (box.max.x - box.min.x) * 0.5f, (box.max.y - box.min.y) * 0.5f, (box.max.z - box.min.z) * 0.5f };

	for (int i = 0; i < 6; i++) {
		// Calculate the signed distance from the center of the box to the plane
		float distToCenter = Vector3DotProduct(frustumPlanes[i].normal, center) + frustumPlanes[i].d;

		// Calculate the projected "radius" of the box onto the plane's normal
		// This is the maximum extent of the box along the normal direction
		float projectedRadius = halfSize.x * fabsf(frustumPlanes[i].normal.x) +
			halfSize.y * fabsf(frustumPlanes[i].normal.y) +
			halfSize.z * fabsf(frustumPlanes[i].normal.z);

		// If the box is entirely on the negative side of the plane (outside), then it's not in the frustum
		if (distToCenter + projectedRadius < 0) {
			return false; // Box is outside this plane, so outside the frustum
		}
		// Optional: Check for fully inside (distToCenter - projectedRadius > 0)
		// For culling, we only need to know if it's potentially visible (intersects or inside)
	}
	return true; // Box intersects or is inside all planes
}



