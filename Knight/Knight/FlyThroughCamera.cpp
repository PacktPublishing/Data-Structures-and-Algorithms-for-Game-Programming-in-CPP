#include "Knight.h"
#include "FlyThroughCamera.h"

FlyThroughCamera::FlyThroughCamera(Scene* Scene, const char* Name, bool IsMainCamera)
    : SceneCamera(Scene, Name, IsMainCamera)
{
    _Camera.projection = CAMERA_PERSPECTIVE;
}

bool FlyThroughCamera::Update(float ElapsedSeconds)
{
    Vector2 delta = GetMouseDelta();

	__super::Update(ElapsedSeconds); // Call base class update

    if (!IsActive)
        return false;

    // Handle mouse rotation
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        // Adjust yaw and pitch based on mouse movement
        yaw += delta.x * sensitivity;
        pitch -= delta.x * sensitivity;

        // Clamp pitch to avoid flipping
        const float pitchLimit = DEG2RAD*45; // Just below 90 degrees
        if (pitch > pitchLimit) pitch = pitchLimit;
        if (pitch < -pitchLimit) pitch = -pitchLimit;
    }

    // Handle zoom
    distance -= GetMouseWheelMove() * zoomSpeed;
    if (distance < 1.0f) distance = 1.0f; // Minimum zoom distance

    // Handle movement using arrow keys
    Vector3 forward = {
        sinf(yaw) ,
        0.0f,
        cosf(yaw)
    };

    Vector3 right = {
        cosf(yaw),
        0.0f,
        -sinf(yaw)
    };

    if (Knight::Instance->Input.IsKeyDown(KEY_UP)) {
        _Camera.target.x -= forward.x * moveSpeed;
        _Camera.target.z -= forward.z * moveSpeed;
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_DOWN)) {
        _Camera.target.x += forward.x * moveSpeed;
        _Camera.target.z += forward.z * moveSpeed;
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_RIGHT)) {
        _Camera.target.x += right.x * moveSpeed;
        _Camera.target.z += right.z * moveSpeed;
    }
    if (Knight::Instance->Input.IsKeyDown(KEY_LEFT)) {
        _Camera.target.x -= right.x * moveSpeed;
        _Camera.target.z -= right.z * moveSpeed;
    }


    // Convert spherical coordinates to Cartesian coordinates
    Vector3 pos;
    pos.x = _Camera.target.x + distance * cosf(pitch) * sinf(yaw);
    pos.y = _Camera.target.y + distance * sinf(pitch);
    pos.z = _Camera.target.z + distance * cosf(pitch) * cosf(yaw);

    //update Camera3D
    _Camera.position = pos;

    return true;
}

void FlyThroughCamera::SetUp(Vector3 target, float dist, float yawAngle, float pitchAngle, float fovY, int projType)
{
    // Set up the camera with a third-person perspective	
    _Camera.fovy = fovY; // Field of view
    _Camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Up direction of the camera
    _Camera.target = target; // Focus on the player
    _Camera.projection = projType;

    yaw = DegreesToRadians(yawAngle);
    pitch = DegreesToRadians(pitchAngle);
    distance = dist;
}

FlyThroughCamera::~FlyThroughCamera()
{
}
