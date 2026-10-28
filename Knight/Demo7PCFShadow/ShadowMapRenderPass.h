#pragma once

#include "Knight.h"

#include "ShadowSceneLight.h"

class ShadowMapRenderPass : public SceneRenderPass
{
	public:

		ShadowMapRenderPass(ShadowSceneLight* l, int id);

		bool Create(Scene *sc) override;
		void Release() override;

		void BeginScene(SceneCamera* cam = NULL) override;
		void Render() override;
		void EndScene() override;
		bool OnAddToRender(Component* pSC, SceneObject* pSO) override;

		int depthTextureId = -1;
		int shadowMapResolution = 1024;

		// Record the light matrices for future use!
	
		ShadowSceneLight* pLight = nullptr;

		Shader shadowShader = { 0 };
		int lightDirLoc = -1;
		int lightColLoc = -1;
		int ambientLoc = -1;		
		int lightVPLoc = -1;
		int shadowMapLoc = -1;
		int receiveShadowLoc = -1;

};
