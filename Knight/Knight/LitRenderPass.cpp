#include "Knight.h"

#include "rlgl.h"

/// <summary>
/// InitLightUniforms - initialize the shader uniform locations for the lights in the scene.
/// </summary>
/// <param name="shader">The shader to setup the lighting uniforms</param>
void LitRenderPass::InitRenderPassUniforms(const Shader& shader)
{
	__super::InitRenderPassUniforms(shader);

	for (int i = 0; i < NUM_MAX_LIGHTS; i++)
	{
		_SceneLightData[i].attnConstLoc = GetShaderLocation(shader, TextFormat("lights[%i].attn_const", i));
		_SceneLightData[i].attnLinearLoc = GetShaderLocation(shader, TextFormat("lights[%i].attn_linear", i));
		_SceneLightData[i].attnQuadLoc = GetShaderLocation(shader, TextFormat("lights[%i].attn_quad", i));
		_SceneLightData[i].colorLoc = GetShaderLocation(shader, TextFormat("lights[%i].color", i));
		_SceneLightData[i].enabledLoc = GetShaderLocation(shader, TextFormat("lights[%i].enabled", i));
		_SceneLightData[i].positionLoc = GetShaderLocation(shader, TextFormat("lights[%i].position", i));
		_SceneLightData[i].targetLoc = GetShaderLocation(shader, TextFormat("lights[%i].target", i));
		_SceneLightData[i].typeLoc = GetShaderLocation(shader, TextFormat("lights[%i].type", i));
	}
	ambientLoc = GetShaderLocation(shader, "ambient");
	if (ambientLoc >= 0) {
		//Note: rlight of Raylib use a hardcoded value of { 0.1f, 0.1f, 0.1f, 1.0f } for ambient in the 
		//this value should be overridden by the scene, if not set, we provided a default value same as Raylib's rlight module 
		SetShaderValue(shader, ambientLoc, pScene->AmbientColor, SHADER_UNIFORM_VEC4);
	}

	shinenessLoc = GetShaderLocation(shader, "materialShininess");
	if (shinenessLoc >= 0) {
		//Note: rlight of Raylib use a hardcoded value of 16.0f for shininess in the 
		//this value should be overridden by the material, if not set, we provided a default value same as Raylib's rlight module 
		SetShaderValue(shader, shinenessLoc, &pScene->DefaultShineness, SHADER_UNIFORM_FLOAT);
	}
}

/// <summary>
/// UpdateLightData - update the light data in the shader if any light is dirty (has changed since last update).
/// </summary>
/// <param name="shader">The shader to accept values from lighting related uniforms</param>
void LitRenderPass::UpdateLightData(const Shader& shader)
{
	for (int i = 0; i < NUM_MAX_LIGHTS; i++)
	{
		LightData* pLightData = &pScene->Lights[i];

		if (pLightData->dirty != false)
		{
			// GLSL bool/int uniforms are uploaded as 32-bit integers. LightData::enabled
			// is a one-byte C++ bool, so passing its address made SetShaderValue read
			// adjacent padding bytes. In MSVC Debug builds those bytes can be non-zero,
			// producing a value other than 1 and disabling the light in kn_lit.fs.
			const int lightEnabled = pLightData->enabled ? 1 : 0;
			SetShaderValue(shader, _SceneLightData[i].enabledLoc, &lightEnabled, SHADER_UNIFORM_INT);
			if (!pLightData->enabled)
			{
				//if the light get disabled, there is no need to update the rest of the light data
				continue;
			}

			//Update light data
			SetShaderValue(shader, _SceneLightData[i].typeLoc, &pLightData->type, SHADER_UNIFORM_INT);
			float position[3] = { pLightData->position.x, pLightData->position.y, pLightData->position.z };
			SetShaderValue(shader, _SceneLightData[i].positionLoc, position, SHADER_UNIFORM_VEC3);
			float target[3] = { pLightData->target.x, pLightData->target.y, pLightData->target.z };
			SetShaderValue(shader, _SceneLightData[i].targetLoc, target, SHADER_UNIFORM_VEC3);
			float color[4] = { (float)pLightData->color.r / (float)255, (float)pLightData->color.g / (float)255,
							   (float)pLightData->color.b / (float)255, (float)pLightData->color.a / (float)255 };
			SetShaderValue(shader, _SceneLightData[i].colorLoc, color, SHADER_UNIFORM_VEC4);
			float attn_const = pLightData->attn_const;
			SetShaderValue(shader, _SceneLightData[i].attnConstLoc, &attn_const, SHADER_UNIFORM_FLOAT);
			float attn_linear = pLightData->attn_linear;
			SetShaderValue(shader, _SceneLightData[i].attnLinearLoc, &attn_linear, SHADER_UNIFORM_FLOAT);
			float attn_quad = pLightData->attn_quad;
			SetShaderValue(shader, _SceneLightData[i].attnQuadLoc, &attn_quad, SHADER_UNIFORM_FLOAT);

			//remove dirty flag
			//it will be caller function's responsibility to set the dirty flag when the light data changes
			//this is to avoid other LitRenderPass inherited classes to miss the light data update (2025/9/11)
			//ideally the code which set up the dirty flag is also responsible to reset the flag   
			//pLightData->dirty = false;
		}
	}
	// Update ambient light value
	SetShaderValue(shader, ambientLoc, pScene->AmbientColor, SHADER_UNIFORM_VEC4);
}

//End of LitRenderPass.cpp
