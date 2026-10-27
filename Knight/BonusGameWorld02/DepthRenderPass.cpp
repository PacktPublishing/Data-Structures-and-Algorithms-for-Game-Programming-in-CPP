#include "Knight.h"

#include "raylib.h"

#include "DepthRenderPass.h"

DepthRenderPass::DepthRenderPass(ShadowSceneLight* l)
{
	pLight = l;
}

/// <summary>
/// DepthRenderPass - Create the depth render pass for shadow mapping
/// </summary>
/// <param name="sc">The Scene to render shadow</param>
/// <returns>True if initialization is successful.</returns>
bool DepthRenderPass::Create(Scene* sc)
{
	__super::Create(sc);

	depthShader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn-lit-depth.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/shadow_depth.fs").c_str());

	InitRenderPassUniforms(depthShader);

	Hints.pOverrideShader = &depthShader;
	Hints.pOverrideCamera = pLight;

	shadowMap = LoadShadowmapRenderTexture(SHADOWMAP_RESOLUTION, SHADOWMAP_RESOLUTION);

	// For the shadowmapping algorithm, we will be rendering everything from the light's point of view
	Camera3D* lightCam = pLight->GetCamera3D();
	lightCam->position = Vector3Scale(pLight->lightDir, -50.0f);
	lightCam->target = Vector3Zero();

	// Use an orthographic projection for directional lights
	lightCam->projection = CAMERA_ORTHOGRAPHIC;
	//Since we use level of detail for shadow map, we can use a smaller view size
	//This improves shadow quality for the area that matters
	lightCam->fovy = 50.0f;   
	lightCam->up = Vector3{ 0.0f, 1.0f, 0.0f };

	return true;
}

/// <summary>
/// Release - Release resources used by the depth render pass
/// </summary>
void DepthRenderPass::Release()
{
	UnloadShadowmapRenderTexture(shadowMap);
	UnloadShader(depthShader);
}

/// <summary>
/// BeginScene - Prepare for rendering the scene to the depth map
/// </summary>
/// <param name="pOverrideCamera">You can pass a customized camera to override the active one in the DepthRenderPass</param>
void DepthRenderPass::BeginScene(SceneCamera* pOverrideCamera)
{
	UpdateLightData(depthShader);

	NumComponentsSkipped = 0;
	pScene->_CurrentRenderPass = this;
	if (Hints.pOverrideCamera != nullptr)
		pActiveCamera = Hints.pOverrideCamera;
	else
		pActiveCamera = pScene->GetMainCameraActor();
	if (pOverrideCamera != nullptr)
		pActiveCamera = pOverrideCamera;
	pScene->ClearRenderQueue();
	BuildRenderQueue(pScene->SceneRoot);

	//Override shader 
	BeginShaderMode(depthShader);
}

/// <summary>
/// Render - Render the scene to the depth map
/// </summary>
void DepthRenderPass::Render()
{
	//render background first
	vector<RenderContext>::iterator bk = pScene->_RenderQueue.Background.begin();
	while (bk != pScene->_RenderQueue.Background.end())
	{
		bk->pComponent->Draw(&Hints);
		++bk;
	}

	//render opauqe geometry from nearest to farest
	multiset<RenderContext, CompareDistanceAscending>::iterator opaque = pScene->_RenderQueue.Geometry.begin();
	while (opaque != pScene->_RenderQueue.Geometry.end())
	{
		BeginBlendMode(opaque->pComponent->blendingMode);
		opaque->pComponent->Draw(&Hints);
		EndBlendMode();
		++opaque;
	}

	//render alpha blend from back to front
	multiset<RenderContext, CompareDistanceDescending>::iterator alpha = pScene->_RenderQueue.AlphaBlending.begin();
	while (alpha != pScene->_RenderQueue.AlphaBlending.end())
	{
		BeginBlendMode(alpha->pComponent->blendingMode);
		alpha->pComponent->Draw(&Hints);
		EndBlendMode();
		++alpha;
	}

	//render overlay at last
	vector<RenderContext>::iterator overlay = pScene->_RenderQueue.Overlay.begin();
	while (overlay != pScene->_RenderQueue.Overlay.end())
	{
		BeginBlendMode(alpha->pComponent->blendingMode);
		overlay->pComponent->Draw(&Hints);
		EndBlendMode();
		++overlay;
	}

}

/// <summary>
/// EndScene - Finish rendering the scene to the depth map
/// </summary>
void DepthRenderPass::EndScene()
{
	EndShaderMode();
	pActiveCamera = nullptr;
	pScene->_CurrentRenderPass = nullptr;
}

/// <summary>
/// OnAddToRender - This method is called by the SceneRenderPass to add a Component to the render queue.
/// </summary>
/// <param name="pSC">The Component candidate to test if we should include it</param>
/// <param name="pSO">The Scene Object the Component is attached to.</param>
/// <returns>True if this Component should be included</returns>
bool DepthRenderPass::OnAddToRender(Component* pSC, SceneObject* pSO)
{
	//If this Component do not cast shadow to other objects in the scene, no need to redner in depth render pass
	if (pSC->castShadow == Component::eShadowCastingType::NoShadow)
		return false;	
	return __super::OnAddToRender(pSC, pSO);
}

/// <summary>
/// BeginShadowMap - Set up rendering to the shadow map
/// </summary>
/// <param name="sc">The Scene</param>
/// <param name="pOverrideCamera">Allow to pass a customized SceneCamera for rendering</param>
void DepthRenderPass::BeginShadowMap(Scene* sc, SceneCamera* pOverrideCamera)
{
	// First, render all objects into the shadowmap
	// The idea is, we record all the objects' depths (as rendered from the light source's point of view) in a buffer
	// Anything that is "visible" to the light is in light, anything that isn't is in shadow
	// We can later use the depth buffer when rendering everything from the player's point of view
	// to determine whether a given point is "visible" to the light
	BeginTextureMode(shadowMap);
	ClearBackground(WHITE);
	BeginMode3D(*pLight->GetCamera3D());
	pLight->lightView = rlGetMatrixModelview();
	pLight->lightProj = rlGetMatrixProjection();
}

/// <summary>
/// EndShadowMap - Finish rendering to the shadow map
/// </summary>
void DepthRenderPass::EndShadowMap()
{
	EndMode3D();
	EndTextureMode();
}

// Unload shadowmap render texture from GPU memory (VRAM)
void DepthRenderPass::UnloadShadowmapRenderTexture(RenderTexture2D target)
{
	if (target.id > 0)
	{
		// NOTE: Depth texture/renderbuffer is automatically
		// queried and deleted before deleting framebuffer
		rlUnloadFramebuffer(target.id);
	}
}

/// <summary>
/// LoadShadowmapRenderTexture - Load a render texture to be used as the shadow map
/// </summary>
/// <param name="width">shadow map width in pixel</param>
/// <param name="height">shadow map height in pixel</param>
/// <returns>The created RenderTexture2D object</returns>
RenderTexture2D DepthRenderPass::LoadShadowmapRenderTexture(int width, int height)
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


void DepthRenderPass::InitRenderPassUniforms(const Shader& shader)
{
	__super::InitRenderPassUniforms(shader);
}

void DepthRenderPass::UpdateLightData(const Shader& shader)
{
	pScene->Lights[0].position = pLight->GetCamera3D()->position;
	pScene->Lights[0].target = pLight->GetCamera3D()->target;
	pScene->Lights[0].type = pLight->lightType;
	pScene->Lights[0].color = pLight->lightColor;
	pScene->Lights[0].enabled = true;
	pScene->Lights[0].dirty = true;
		
	__super::UpdateLightData(shader);
}

//End of DepthRenderPass.cpp