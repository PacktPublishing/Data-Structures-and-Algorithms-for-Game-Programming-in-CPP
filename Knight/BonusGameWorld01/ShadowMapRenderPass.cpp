//Implementation of shadow map render pass for rendering scene with shadows
#include "Knight.h"

#include "rlgl.h"

#include "ShadowMapRenderPass.h"

ShadowMapRenderPass::ShadowMapRenderPass(ShadowSceneLight* l, int id)
{
	pLight = l;
	depthTextureId = id;
}

bool ShadowMapRenderPass::Create(Scene *sc)
{
	__super::Create(sc);

	shadowShader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/shadowmap.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn-lit-sm-pcf.fs").c_str());
	lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
	lightColLoc = GetShaderLocation(shadowShader, "lightColor");
	shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");
	ambientLoc = GetShaderLocation(shadowShader, "ambient");
	lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
	shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");
	receiveShadowLoc = GetShaderLocation(shadowShader, "receiveShadow");

	Hints.pOverrideShader = &shadowShader;

	Vector4 lightColorNormalized = ColorNormalize(pLight->lightColor);
	Vector4 ambientColorNormalized = ColorNormalize(pLight->lightAmbient);

	SetShaderValue(shadowShader, lightDirLoc, &pLight->lightDir, SHADER_UNIFORM_VEC3);
	SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
	SetShaderValue(shadowShader, ambientLoc, &ambientColorNormalized, SHADER_UNIFORM_VEC4);
	SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);

	return true;
}

void ShadowMapRenderPass::Release()
{
	UnloadShader(shadowShader);
}

bool ShadowMapRenderPass::OnAddToRender(Component* pSC, SceneObject* pSO)
{
	float dist2 = 0;

	SceneActor* pActor = dynamic_cast<SceneActor*>(pSO);

	if (pActor != nullptr) {
		Vector3 pos = pActor->Position;
		if (pActiveCamera != NULL)
			dist2 = Vector3DistanceSqr(pos, pActiveCamera->GetPosition());
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

void ShadowMapRenderPass::BeginScene(SceneCamera* pOverrideCamera)
{
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
	SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
	SetShaderValue(shadowShader, ambientLoc, &ambientColorNormalized, SHADER_UNIFORM_VEC4);
}

void ShadowMapRenderPass::EndScene()
{
	pActiveCamera = nullptr;
	pScene->_CurrentRenderPass = nullptr;
	EndShaderMode();
}

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
