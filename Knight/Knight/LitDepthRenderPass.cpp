#include "Knight.h"

#include "rlgl.h"

#include "LitDepthRenderPass.h"

LitDepthRenderPass::LitDepthRenderPass()
{
}

bool LitDepthRenderPass::Create(Scene* sc)
{
	__super::Create(sc);

	depthShader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn_depth.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn_depth.fs").c_str());

	Hints.pOverrideShader = &depthShader;

	//Create shadowmap render textures
	for(int i=0;i< NUM_MAX_LIGHTS; ++i)
	{
		ShadowMaps.push_back(LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION));
		// We only care about the depth buffer, so no need for a color attachment
		// and disable filtering/mipmaps on the depth texture to keep it sharp
		SetTextureFilter(ShadowMaps[i].depth, TEXTURE_FILTER_POINT);
		SetTextureWrap(ShadowMaps[i].depth, TEXTURE_WRAP_CLAMP);
	}

	for (int i = 0; i < NUM_MAX_LIGHTS; ++i) {
		ShadowMapsLocs[i] = GetShaderLocation(depthShader, TextFormat("shadowMaps[%d]", i));
	}

	pLightCamera = new OrthogonalCamera(pScene, "Light Camera", false);

	return true;
}

void LitDepthRenderPass::Release()
{
	for(int i = 0; i < NUM_MAX_LIGHTS; ++i)
	{
		if (ShadowMaps[i].id > 0)
		{
			UnloadShadowmapRenderTexture(ShadowMaps[i]);
		}
	}
	UnloadShader(depthShader);
}

void LitDepthRenderPass::BeginScene(SceneCamera* pOverrideCamera)
{
	pScene->_CurrentRenderPass = this;

	pScene->ClearRenderQueue();
	BuildRenderQueue(pScene->SceneRoot);

	//Override shader 
	BeginShaderMode(depthShader);
}

void LitDepthRenderPass::Render()
{
	pActiveCamera = pLightCamera;

	for(int i=0;i<NUM_MAX_LIGHTS; ++i)
	{
		if (pScene->Lights[i].enabled) 
		{
			pLightCamera->SetUp(pScene->Lights[i].position, pScene->Lights[i].target, 45);

			BeginTextureMode(ShadowMaps[i]);

			// Clear the depth buffer. VERY IMPORTANT!
		    // Clear the depth buffer to the farthest value (1.0) so objects behind occluders are marked as in shadow.
			rlClearColor(0, 0, 0, 0); // Clear color buffer (not used, but good practice)
			rlClearScreenBuffers();
			// Set the shadow map texture for the current light
			SetShaderValueTexture(depthShader, ShadowMapsLocs[i], ShadowMaps[i].depth);

			BeginMode3D(*pLightCamera->GetCamera3D());
			SceneRenderPass::Render();
			EndMode3D();

			EndTextureMode();
		}
	}
	
}

void LitDepthRenderPass::EndScene()
{
	EndShaderMode();
	pActiveCamera = nullptr;
	pScene->_CurrentRenderPass = nullptr;
}

bool LitDepthRenderPass::OnAddToRender(Component* pSC, SceneObject* pSO)
{
	//If this Component do not cast shadow to other objects in the scene, no need to redner in depth render pass
	if (pSC->castShadow == Component::eShadowCastingType::NoShadow)
		return false;
	return __super::OnAddToRender(pSC, pSO);
}

// Unload shadowmap render texture from GPU memory (VRAM)
void LitDepthRenderPass::UnloadShadowmapRenderTexture(RenderTexture2D target)
{
	if (target.id > 0)
	{
		// NOTE: Depth texture/renderbuffer is automatically
		// queried and deleted before deleting framebuffer
		rlUnloadFramebuffer(target.id);
	}
}

RenderTexture2D LitDepthRenderPass::LoadShadowmapRenderTexture(int width, int height)
{
	RenderTexture2D target = { 0 };

	target.id = rlLoadFramebuffer(); // Load an empty framebuffer
	target.texture.width = width;
	target.texture.height = height;

	if (target.id > 0)
	{
		rlEnableFramebuffer(target.id);

		// Create depth texture
		// We don't need a color texture for the shadowmap
		target.depth.id = rlLoadTextureDepth(width, height, false);
		target.depth.width = width;
		target.depth.height = height;
		target.depth.format = 19;       //DEPTH_COMPONENT_24BIT
		target.depth.mipmaps = 1;

		// Attach depth texture to FBO
		rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);

		// Check if fbo is complete with attachments (valid)
		if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

		rlDisableFramebuffer();
	}
	else
		return { 0 };

	return target;
}

// Function to calculate a light's projection matrix
Matrix GetLightProjection(LightData light) {
	if (light.type == 0) {
		// Orthographic projection for directional lights (like the sun)
		// Adjust the orthographic frustum size as needed to cover your scene
		return MatrixOrtho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 100.0f);
	}
	else { // LIGHT_POINT
		// Perspective projection for point lights
		return MatrixPerspective(DEG2RAD * 90.0f, 1.0f, 0.1f, 100.0f);
	}
}

// Function to calculate a light's view matrix
Matrix GetLightView(LightData light) {
	return MatrixLookAt(light.position, light.target, Vector3 { 0.0f, 1.0f, 0.0f });
}
