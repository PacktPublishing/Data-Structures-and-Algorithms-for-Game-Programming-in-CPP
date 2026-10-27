#include "Knight.h"

#include "rlgl.h"

bool ForwardRenderPass::Create(Scene* sc)
{
	__super::Create(sc);

	//default forward rendering pipeline use simple lighting model
	LightShader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn_lit.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/kn_lit.fs").c_str());
	//initialize light uniform locations
	InitRenderPassUniforms(LightShader);

	Hints.pOverrideShader = &LightShader;

	return true;
}

void ForwardRenderPass::Release()
{
}

void ForwardRenderPass::BeginScene(SceneCamera *pOverrideCamera)
{
	UpdateLightData(LightShader);

	pActiveCamera = pScene->GetMainCameraActor();
	if (pOverrideCamera != nullptr)
		pActiveCamera = pOverrideCamera;
	pScene->_CurrentRenderPass = this;
	pScene->ClearRenderQueue();
	BuildRenderQueue(pScene->SceneRoot);	
}

void ForwardRenderPass::EndScene()
{
	pActiveCamera = NULL;
	pScene->_CurrentRenderPass = nullptr;
}

//End of ForwardRenderPass.cpp
