#pragma once

#include "Knight.h"

#include "ShadowSceneLight.h"

#define SHADOWMAP_RESOLUTION 2048

class DepthRenderPass : public LitRenderPass
{
public:

	DepthRenderPass(ShadowSceneLight* l);

	bool Create(Scene* sc) override;
	void Release() override;

	void BeginScene(SceneCamera* cam = NULL) override;
	void Render() override;
	void EndScene() override;
	bool OnAddToRender(Component* pSC, SceneObject* pSO) override;

	void BeginShadowMap(Scene* sc, SceneCamera* cam = NULL);
	void EndShadowMap();

	RenderTexture2D shadowMap = { 0 };
	int shadowMapResolution = SHADOWMAP_RESOLUTION;

	Shader depthShader = { 0 };
	int alphaTestLoc = -1;

	ShadowSceneLight* pLight = nullptr;

protected:
	void InitRenderPassUniforms(const Shader&) override;
	void UpdateLightData(const Shader&) override;

	RenderTexture2D LoadShadowmapRenderTexture(int width, int height);
	void UnloadShadowmapRenderTexture(RenderTexture2D target);
};

