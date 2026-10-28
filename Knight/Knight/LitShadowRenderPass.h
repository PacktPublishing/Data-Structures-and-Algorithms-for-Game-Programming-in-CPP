#pragma once

#include "LitRenderPass.h"
#include "LitDepthRenderPass.h"

class LitShadowRenderPass : public LitRenderPass
{
public:

	LitShadowRenderPass(LitDepthRenderPass *pPass);

	bool Create(Scene* sc) override;
	void Release() override;

	void BeginScene(SceneCamera* cam = NULL) override;
	void Render() override;
	void EndScene() override;
	bool OnAddToRender(Component* pSC, SceneObject* pSO) override;

	int depthTextureId = -1;
	int shadowMapResolution = SHADOWMAP_RESOLUTION;

	LitDepthRenderPass* pLitDepthPass = nullptr;

	Shader shadowShader = { 0 };

	int ShadowMapsLocs[NUM_MAX_LIGHTS] = { 0 };
	int lightSpaceMatricesLocs[NUM_MAX_LIGHTS] = { 0 };

	int viewPosLoc = -1;
	//int ambientLoc = -1; //from SceneRenderPass.h, no need to redefine here

	//uniforms for material properties
	//int shinessLoc = -1;  // from SceneRenderPass.h, no need to redefine here
	int specularColorLoc = -1;

	int lightDirLoc = -1;
	int lightColLoc = -1;

	int lightVPLoc = -1;
	//int shadowMapLoc = -1;

protected:
	Matrix GetLightProjection(const LightData& light);
	Matrix GetLightView(const LightData& light);
}; 

