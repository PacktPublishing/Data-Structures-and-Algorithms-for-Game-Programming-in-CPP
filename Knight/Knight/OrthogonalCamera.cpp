#include "OrthogonalCamera.h"
#include "Knight.h"
#include "Scene.h"

OrthogonalCamera::OrthogonalCamera(Scene* Scene, const char* Name, bool IsMainCamera)
	: SceneCamera(Scene, Name, IsMainCamera)
{
	_Camera.projection = CAMERA_ORTHOGRAPHIC;
	CameraMode = CAMERA_CUSTOM; // Set to custom mode to allow manual control
}

void OrthogonalCamera::SetUp(Vector3 pos, Vector3 target, float zoom)
{
    // Set up the camera with a third-person perspective	
    _Camera.position = pos; // Initial position of the camera
    _Camera.fovy = zoom; // Field of view
    _Camera.up = Vector3{ 0.0f, 1.0f, 0.0f }; // Up direction of the camera
    _Camera.target = target; // Focus on the player
}

bool OrthogonalCamera::Update(float ElapsedSeconds)
{
    if (!IsActive)
        return false;

	__super::Update(ElapsedSeconds); // Call base class update

    if (!_ProcessInput)
		return true; // If input processing is disabled, skip further updates

    // Zoom control with mouse wheel
    cameraZoom -= GetMouseWheelMove() * 0.5f;
    if (cameraZoom < 2.0f) cameraZoom = 2.0f;   // Minimum zoom
    if (cameraZoom > 50.0f) cameraZoom = 50.0f; // Maximum zoom
    _Camera.fovy = cameraZoom;

    //Calculate the camera's position 
    _Camera.position.x = _Camera.target.x - sinf(cameraHorizontalAngleShift) * cameraZoom;
    _Camera.position.z = _Camera.target.z - cosf(cameraHorizontalAngleShift) * cameraZoom;
    _Camera.position.y = cameraZoom + cameraVerticalOffset;

    // Panning with arrow keys
    if (Knight::Instance->Input.IsKeyDown(KEY_UP)) _Camera.target.z -= panSpeed;
    if (Knight::Instance->Input.IsKeyDown(KEY_DOWN)) _Camera.target.z += panSpeed;
    if (Knight::Instance->Input.IsKeyDown(KEY_LEFT)) _Camera.target.x -= panSpeed;
    if (Knight::Instance->Input.IsKeyDown(KEY_RIGHT)) _Camera.target.x += panSpeed;

    // Rotate the camera around the player when right mouse button is held
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        cameraHorizontalAngleShift += GetMouseDelta().x * 0.01f;
        cameraVerticalOffset += GetMouseDelta().y * 0.01f;
    }

    return true;
}

OrthogonalCamera::~OrthogonalCamera()
{

}

