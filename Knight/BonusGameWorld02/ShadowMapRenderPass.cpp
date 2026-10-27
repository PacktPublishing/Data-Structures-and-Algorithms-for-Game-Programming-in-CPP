#include "Knight.h"

#include "rlgl.h"

#include "ShadowMapRenderPass.h"

/// <summary>
/// ShadowMapRenderPass - constructor
/// </summary>
/// <param name="l">The shadow scene light</param>
/// <param name="id">The Texture id of depth render to texture.</param>
ShadowMapRenderPass::ShadowMapRenderPass(ShadowSceneLight* l, int id)
{
	pLight = l;
	depthTextureId = id;
}

/// <summary>
/// Create - Override standard create function to setup shadow map shader and related shader locations
/// </summary>
/// <param name="sc">The Scene</param>
/// <returns>True if initialization is successful.</returns>
bool ShadowMapRenderPass::Create(Scene *sc)
{
	__super::Create(sc);

	shadowShader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/shadowmap.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn-lit4-sm-pcf.fs").c_str());
	InitRenderPassUniforms(shadowShader);

	Hints.pOverrideShader = &shadowShader;

	Vector4 lightColorNormalized = ColorNormalize(pLight->lightColor);
	Vector4 ambientColorNormalized = ColorNormalize(pLight->lightAmbient);

	SetShaderValue(shadowShader, lightDirLoc, &pLight->lightDir, SHADER_UNIFORM_VEC3);
	SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
	SetShaderValue(shadowShader, ambientLoc, &ambientColorNormalized, SHADER_UNIFORM_VEC4);
	SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);

	return true;
}

/// Release - Override standard release function to unload shadow map shader
void ShadowMapRenderPass::Release()
{
	UnloadShader(shadowShader);
}

/// <summary>
/// OnAddToRender - Override standard OnAddToRender function to skip adding components that do not cast shadow
/// </summary>
/// <param name="pSC">The Component candiate to add</param>
/// <param name="pSO">The SceneObject the Component is attached to</param>
/// <returns>True if we should include this Component candidate to render</returns>
bool ShadowMapRenderPass::OnAddToRender(Component* pSC, SceneObject* pSO)
{
	float dist2 = 0;

	SceneActor* pActor = dynamic_cast<SceneActor*>(pSO);

	if (pActor != nullptr) {
		Vector3 pos = pActor->Position;
		if (pActiveCamera != NULL) 
		{
			//use bounding box distance if available, otherwise fallback to actor position distance
			if (IsBoundingBoxValid(pActor->WorldBoundingBox)) {
				dist2 = PointToBoxDistanceSqr(pActiveCamera->GetPosition(), pActor->WorldBoundingBox);
			} else {
				//fallback to use actor position if bounding box is not valid
				dist2 = Vector3DistanceSqr(pos, pActiveCamera->GetPosition());
			}
		}
	}

	switch (pSC->renderQueue)
	{
		case Component::eRenderQueueType::Background:
			pScene->_RenderQueue.Background.push_back(RenderContext{ pSC, dist2 });
			break;
		case Component::eRenderQueueType::Geometry:
			pScene->_RenderQueue.Geometry.insert(RenderContext{ pSC, dist2 });
			break;
		case Component::eRenderQueueType::AlphaBlend:
			pScene->_RenderQueue.AlphaBlending.insert(RenderContext{ pSC, dist2 });
			break;
		case Component::eRenderQueueType::Overlay:
			pScene->_RenderQueue.Overlay.push_back(RenderContext{ pSC, dist2 });
			break;
	}

	return true;
}

/// <summary>
/// BeginScene - Override standard BeginScene function to setup shadow map rendering state
/// </summary>
/// <param name="pOverrideCamera">Customzied SceneCamera to render shadow, if any.</param>
void ShadowMapRenderPass::BeginScene(SceneCamera* pOverrideCamera)
{
	UpdateLightData(shadowShader);

	NumComponentsSkipped = 0;
	pScene->_CurrentRenderPass = this;

	pLight->lightViewProj = MatrixMultiply(pLight->lightView, pLight->lightProj);
	SetShaderValueMatrix(shadowShader, lightVPLoc, pLight->lightViewProj);

	//Make shadow map referenced texture as 5th texture, the first four textures are commonly used by other effects
	int slot = 4; 
	rlActiveTextureSlot(slot);
	rlEnableTexture(depthTextureId);
	rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

	pActiveCamera = pScene->GetMainCameraActor();
	if (pOverrideCamera != nullptr)
		pActiveCamera = pOverrideCamera;
	pScene->ClearRenderQueue();
	BuildRenderQueue(pScene->SceneRoot);

	//update lighting changes (if any)
	Vector4 lightColorNormalized = ColorNormalize(pLight->lightColor);
	Vector4 ambientColorNormalized = ColorNormalize(pLight->lightAmbient);
	//SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
	SetShaderValue(shadowShader, ambientLoc, &ambientColorNormalized, SHADER_UNIFORM_VEC4);
}

/// <summary>
/// EndScene - Override standard EndScene function to reset shadow map rendering state
/// </summary>
void ShadowMapRenderPass::EndScene()
{
	pActiveCamera = nullptr;
	pScene->_CurrentRenderPass = nullptr;
	EndShaderMode();
}

/// <summary>
/// Render - Override standard Render function to render all components in the render queue
/// </summary>
void ShadowMapRenderPass::Render()
{
	//render background first
	vector<RenderContext>::iterator bk = pScene->_RenderQueue.Background.begin();
	while (bk != pScene->_RenderQueue.Background.end()) {
		int receiveShadow = bk->pComponent->receiveShadow ? 1 : 0;
		SetShaderValue(shadowShader, receiveShadowLoc, &receiveShadow, SHADER_UNIFORM_INT);
		bk->pComponent->Draw(&Hints);
		++bk;
	}

	//render opauqe geometry from nearest to farest
	multiset<RenderContext, CompareDistanceAscending>::iterator opaque = pScene->_RenderQueue.Geometry.begin();
	while (opaque != pScene->_RenderQueue.Geometry.end()) {
		int receiveShadow = opaque->pComponent->receiveShadow ? 1 : 0;
		SetShaderValue(shadowShader, receiveShadowLoc, &receiveShadow, SHADER_UNIFORM_INT);
		opaque->pComponent->Draw(&Hints);
		++opaque;
	}
	
	//render alpha blend
	multiset<RenderContext, CompareDistanceDescending>::iterator alpha = pScene->_RenderQueue.AlphaBlending.begin();
	while (alpha != pScene->_RenderQueue.AlphaBlending.end()) {
		rlDisableDepthMask();
		rlDisableBackfaceCulling();
		BeginBlendMode(alpha->pComponent->blendingMode);
		int receiveShadow = alpha->pComponent->receiveShadow ? 1 : 0;
		SetShaderValue(shadowShader, receiveShadowLoc, &receiveShadow, SHADER_UNIFORM_INT);
		alpha->pComponent->Draw(&Hints);
		EndBlendMode();
		rlEnableDepthMask();
		rlEnableBackfaceCulling();
		++alpha;
	}
	
	//render overlay first
	vector<RenderContext>::iterator overlay = pScene->_RenderQueue.Overlay.begin();
	while (overlay != pScene->_RenderQueue.Overlay.end()) {
		int receiveShadow = overlay->pComponent->receiveShadow ? 1 : 0;
		SetShaderValue(shadowShader, receiveShadowLoc, &receiveShadow, SHADER_UNIFORM_INT);
		overlay->pComponent->Draw(&Hints);
		++overlay;
	}	
}

/// <summary>
/// InitRenderPassUniforms - Override function to initialize the shader uniforms
/// </summary>
/// <param name="shader">The shader to communicate with this unforms</param>
void ShadowMapRenderPass::InitRenderPassUniforms(const Shader& shader)
{
	__super::InitRenderPassUniforms(shader);

	lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
	lightColLoc = GetShaderLocation(shadowShader, "lightColor");
	shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
	ambientLoc = GetShaderLocation(shadowShader, "ambient");
	lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
	shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
	receiveShadowLoc = GetShaderLocation(shadowShader, "receiveShadow");
}

/// <summary>
/// UpdateLightData - Overrided function to take primary lighting data from ShadowSceneLight, and leave other light alone.
/// </summary>
/// <param name="shader">The shader to handle lighting</param>
void ShadowMapRenderPass::UpdateLightData(const Shader& shader)
{
	pScene->Lights[0].position = pLight->GetCamera3D()->position;
	pScene->Lights[0].target = pLight->GetCamera3D()->target;
	pScene->Lights[0].type = pLight->lightType;
	pScene->Lights[0].color = pLight->lightColor;
	pScene->Lights[0].enabled = true;
 	pScene->Lights[0].dirty = true;

	__super::UpdateLightData(shader);
}

//End of ShadowMapRenderPass.cpp