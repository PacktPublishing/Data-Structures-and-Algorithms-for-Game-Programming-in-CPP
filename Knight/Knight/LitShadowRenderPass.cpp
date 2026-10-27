#include "Knight.h"

#include "rlgl.h"

#include "LitShadowRenderPass.h"

LitShadowRenderPass::LitShadowRenderPass(LitDepthRenderPass *pPass)
{
	pLitDepthPass = pPass;
}

bool LitShadowRenderPass::Create(Scene* sc)
{
	__super::Create(sc);

	shadowShader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn-sm.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn-sm-pcf.fs").c_str());
	InitLightUniforms(shadowShader);

	for (int i = 0; i < NUM_MAX_LIGHTS; ++i) {
		ShadowMapsLocs[i] = GetShaderLocation(shadowShader, TextFormat("shadowMaps[%d]", i));
		lightSpaceMatricesLocs[i] = GetShaderLocation(shadowShader, TextFormat("lightSpaceMatrices[%d]", i));
	}

	viewPosLoc = GetShaderLocation(shadowShader, "viewPos");
	ambientLoc = GetShaderLocation(shadowShader, "ambient");
	shinenessLoc = GetShaderLocation(shadowShader, "materialShininess");
	specularColorLoc = GetShaderLocation(shadowShader, "materialSpecularColor");

	// Tell the shader which texture unit corresponds to which shadow map
	for (int i = 0; i < NUM_MAX_LIGHTS; ++i) {
		int unitid = 4 + i; // Start from texture unit 4 for shadow maps
		SetShaderValue(shadowShader, ShadowMapsLocs[i], &unitid, SHADER_UNIFORM_INT);
	}

	//lightDirLoc = GetShaderLocation(shadowShader, "lightDir");
	//lightColLoc = GetShaderLocation(shadowShader, "lightColor");
	//shadowShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shadowShader, "viewPos");

	//lightVPLoc = GetShaderLocation(shadowShader, "lightVP");
	//shadowMapLoc = GetShaderLocation(shadowShader, "shadowMap");

	Hints.pOverrideShader = &shadowShader;

	//Vector4 lightColorNormalized = ColorNormalize(pLight->lightColor);
	//Vector4 ambientColorNormalized = ColorNormalize(pLight->lightAmbient);

	//SetShaderValue(shadowShader, lightDirLoc, &pLight->lightDir, SHADER_UNIFORM_VEC3);
	//SetShaderValue(shadowShader, lightColLoc, &lightColorNormalized, SHADER_UNIFORM_VEC4);
	//SetShaderValue(shadowShader, ambientLoc, &ambientColorNormalized, SHADER_UNIFORM_VEC4);
	//SetShaderValue(shadowShader, GetShaderLocation(shadowShader, "shadowMapResolution"), &shadowMapResolution, SHADER_UNIFORM_INT);

	return true;
}

void LitShadowRenderPass::Release()
{
	UnloadShader(shadowShader);
}

bool LitShadowRenderPass::OnAddToRender(Component* pSC, SceneObject* pSO)
{
	float dist2 = 0;

	SceneActor* pActor = dynamic_cast<SceneActor*>(pSO);

	if (pActor != nullptr) 
	{
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

void LitShadowRenderPass::BeginScene(SceneCamera* pOverrideCamera)
{
	//pLight->lightViewProj = MatrixMultiply(pLight->lightView, pLight->lightProj);
	//SetShaderValueMatrix(shadowShader, lightVPLoc, pLight->lightViewProj);

	//Make shadow map referenced texture as 5th texture, the first four textures are commonly used by other effects
	int slot = 4;
	rlActiveTextureSlot(slot);
	rlEnableTexture(depthTextureId);
	//rlSetUniform(shadowMapLoc, &slot, SHADER_UNIFORM_INT, 1);

	pScene->_CurrentRenderPass = this;
	pActiveCamera = pScene->GetMainCameraActor();
	if (pOverrideCamera != nullptr)
		pActiveCamera = pOverrideCamera;
	pScene->ClearRenderQueue();
	BuildRenderQueue(pScene->SceneRoot);

	// Set shader uniforms for the light shader (once)
	SetShaderValue(shadowShader, viewPosLoc, &pActiveCamera->GetCamera3D()->position, SHADER_UNIFORM_VEC3);
	//apply default shineness
	SetShaderValue(shadowShader, shinenessLoc, &pScene->DefaultShineness, SHADER_UNIFORM_FLOAT);
	//apply defalt specular color
	SetShaderValue(shadowShader, specularColorLoc, pScene->SpecularColor, SHADER_UNIFORM_VEC4);

	UpdateLightData(shadowShader);

	//activate shadow textures and light space matrices
	for(int i = 0; i < NUM_MAX_LIGHTS; ++i) 
	{
		if (pScene->Lights[i].enabled) 
		{
			rlActiveTextureSlot(i+4); // Use a different texture unit for each map
			rlEnableTexture(pLitDepthPass->ShadowMaps[i].depth.id);

			// Pass the light-space matrix for shadow calculation
			Matrix lightProjection = GetLightProjection(pScene->Lights[i]);
			Matrix lightView = GetLightView(pScene->Lights[i]);
			Matrix lightSpaceMatrix = MatrixMultiply(lightView, lightProjection);
			SetShaderValueMatrix(shadowShader, lightSpaceMatricesLocs[i], lightSpaceMatrix);
		}
	}
}

void LitShadowRenderPass::EndScene()
{
	pActiveCamera = nullptr;
	pScene->_CurrentRenderPass = nullptr;
	EndShaderMode();
}

void LitShadowRenderPass::Render()
{
	//render background first
	vector<RenderContext>::iterator bk = pScene->_RenderQueue.Background.begin();
	while (bk != pScene->_RenderQueue.Background.end()) {
		bk->pComponent->Draw(&Hints);
		++bk;
	}

	//render opauqe geometry from nearest to farest
	multiset<RenderContext, CompareDistanceAscending>::iterator opaque = pScene->_RenderQueue.Geometry.begin();
	while (opaque != pScene->_RenderQueue.Geometry.end()) {
		opaque->pComponent->Draw(&Hints);
		++opaque;
	}

	//render alpha blend
	multiset<RenderContext, CompareDistanceDescending>::iterator alpha = pScene->_RenderQueue.AlphaBlending.begin();
	while (alpha != pScene->_RenderQueue.AlphaBlending.end()) {
		rlDisableDepthMask();
		rlDisableBackfaceCulling();
		BeginBlendMode(alpha->pComponent->blendingMode);
		alpha->pComponent->Draw(&Hints);
		EndBlendMode();
		rlEnableDepthMask();
		rlEnableBackfaceCulling();
		++alpha;
	}

	//render overlay first
	vector<RenderContext>::iterator overlay = pScene->_RenderQueue.Overlay.begin();
	while (overlay != pScene->_RenderQueue.Overlay.end()) {
		overlay->pComponent->Draw(&Hints);
		++overlay;
	}
}

// Function to calculate a light's projection matrix
Matrix LitShadowRenderPass::GetLightProjection(const LightData& light)
{
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
Matrix LitShadowRenderPass::GetLightView(const LightData& light)
{
	return MatrixLookAt(light.position, light.target, Vector3 { 0.0f, 1.0f, 0.0f });
}
