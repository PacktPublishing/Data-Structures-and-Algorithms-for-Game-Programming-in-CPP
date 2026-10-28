#include "Demo7Skybox.h"

static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

int main(int argc, char* argv[])
{
	Demo7Skybox* KnightDemo7SkyBox = new Demo7Skybox();

	KnightDemo7SkyBox->Start();
	KnightDemo7SkyBox->GameLoop();

	delete KnightDemo7SkyBox;
	return 0;
}

void Demo7Skybox::Start()
{
	//Initialize Knight Engine with a default scene and camera
	__super::Start();

	Config.ShowFPS = true;

	pMainCamera = _Scene->CreateSceneObject<PerspectiveCamera>("Main Camera");
	pMainCamera->SetFovY(45.0f);
	pMainCamera->SetPosition(Vector3{ 4, 1, 4 });
	pMainCamera->SetLookAtPosition(Vector3{ 0, 0.0f, 0 });
	pMainCamera->CameraMode = CAMERA_FIRST_PERSON;

	//pSkyBox = new SkyboxComponent();
	pSkyBox = pMainCamera->CreateAndAddComponent<SkyboxComponent>();
	pSkyBox->CreateFromFile((Config.ResourcesBasePath + "textures/skybox2.png").c_str(), CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE, 20.0f, false);

	//Place player
	Actor = _Scene->CreateSceneObject<SceneActor>("Player");
	Actor->Scale = Vector3{ 0.3f, 0.3f, 0.3f};
	Actor->Position = Vector3{ 0.f,0.0f,0.f };
	Actor->Rotation = Vector3{ 0,0,0 };
	ModelComponent* animPlayerComponent = Actor->CreateAndAddComponent<ModelComponent>();
	animPlayerComponent->Load3DModel((Config.ResourcesBasePath + "models/gltf/robot.glb").c_str());
	animPlayerComponent->SetAnimation(6);
}

void Demo7Skybox::Update(float ElapsedSeconds)
{
	UpdateCamera(pMainCamera->GetCamera3D(), CAMERA_FREE);
	if (IsKeyDown(KEY_W)) {
		// Move player forward based on their rotation
		Actor->Position.x += sinf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
		Actor->Position.z += cosf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_S)) {
		// Move player backward based on their rotation
		Actor->Position.x -= sinf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
		Actor->Position.z -= cosf(DegreesToRadians(Actor->Rotation.y)) * 0.1f;
	}
	if (IsKeyDown(KEY_A)) {
		Actor->Rotation.y += 1;  // Rotate left
	}
	if (IsKeyDown(KEY_D)) {
		Actor->Rotation.y -= 1;  // Rotate right
	}

	__super::Update(ElapsedSeconds);

	pSkyBox->Update(ElapsedSeconds);
}

void Demo7Skybox::DrawFrame()
{
	pSkyBox->Draw();
	__super::DrawFrame();
	DrawGrid(10, 1.0f);
}

void Demo7Skybox::DrawGUI()
{
	DrawText("Move mouse to rotate the camera!", 10, 50, 30, WHITE);
}

// Generate cubemap texture from HDR texture
static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format)
{
	TextureCubemap cubemap = { 0 };

	rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

	// Setup framebuffer
	unsigned int rbo = rlLoadTextureDepth(size, size, true);
	cubemap.id = rlLoadTextureCubemap(0, size, format, 1);

	unsigned int fbo = rlLoadFramebuffer();
	rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
	rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

	// Check if framebuffer is complete with attachments (valid)
	if (rlFramebufferComplete(fbo)) TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
	//------------------------------------------------------------------------------------------

	// Draw to framebuffer. Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6 faces)
	rlEnableShader(shader.id);

	// Define projection matrix and send it to shader
	Matrix matFboProjection = MatrixPerspective(90.0 * DEG2RAD, 1.0, rlGetCullDistanceNear(), rlGetCullDistanceFar());
	rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

	// Define view matrix for every side of the cubemap
	Matrix fboViews[6] = {
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 1.0f,  0.0f,  0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { -1.0f,  0.0f,  0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f,  1.0f,  0.0f }, Vector3 { 0.0f,  0.0f,  1.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f, -1.0f,  0.0f }, Vector3 { 0.0f,  0.0f, -1.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f,  0.0f,  1.0f }, Vector3 { 0.0f, -1.0f,  0.0f }),
		MatrixLookAt(Vector3 { 0.0f, 0.0f, 0.0f }, Vector3 { 0.0f,  0.0f, -1.0f }, Vector3 { 0.0f, -1.0f,  0.0f })
	};

	rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions

	// Activate and enable texture for drawing to cubemap faces
	rlActiveTextureSlot(0);
	rlEnableTexture(panorama.id);

	for (int i = 0; i < 6; i++)
	{
		// Set the view matrix for the current cube face
		rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);

		// Select the current cubemap face attachment for the fbo
		// WARNING: This function by default enables->attach->disables fbo!!!
		rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
		rlEnableFramebuffer(fbo);

		// Load and draw a cube, it uses the current enabled texture
		rlClearScreenBuffers();
		rlLoadDrawCube();
	}

	// STEP 3: Unload framebuffer and reset state
	rlDisableShader();          // Unbind shader
	rlDisableTexture();         // Unbind texture
	rlDisableFramebuffer();     // Unbind framebuffer
	rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

	// Reset viewport dimensions to default
	rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
	rlEnableBackfaceCulling();

	cubemap.width = size;
	cubemap.height = size;
	cubemap.mipmaps = 1;
	cubemap.format = format;

	return cubemap;
}

void Demo7Skybox::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();

	UnloadFont(_Font);
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}
