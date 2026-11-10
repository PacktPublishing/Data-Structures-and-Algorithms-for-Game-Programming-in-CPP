#include "SkyboxComponent.h"
#include "Knight.h"

SkyboxComponent::SkyboxComponent()
{
	// Use raylib-owned mesh memory. The previous custom mesh pointed at local
	// stack arrays after GenInwardCube() returned, which is undefined behavior
	// and failed on some Android GPU/driver combinations.
	cube = GenMeshCube(1.0f, 1.0f, 1.0f);
	skybox = LoadModelFromMesh(cube);
}
SkyboxComponent::~SkyboxComponent()
{
	//UnloadShader(skybox.materials[0].shader);
	//UnloadTexture(skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture);
	//UnloadModel(skybox);
}

bool SkyboxComponent::CreateFromFile(const char* pSkyboxTexFilePath, CubemapLayout layout, float skyboxScale, bool useHdr)
{
	scale = skyboxScale;
	useHDR = useHdr;

	// Load skybox shader and set required locations
	// NOTE: Some locations are automatically set at shader loading
	skybox.materials[0].shader = LoadShader((Knight::Instance->Config.ResourcesBasePath + KNIGHT_SHADER_DIRECTORY "skybox.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + KNIGHT_SHADER_DIRECTORY "skybox.fs").c_str());

	int val1[1] = { MATERIAL_MAP_CUBEMAP };
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), val1, SHADER_UNIFORM_INT);
	int val2[1] = { useHDR ? 1 : 0 };
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "doGamma"), val2, SHADER_UNIFORM_INT);
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "vflipped"), val2, SHADER_UNIFORM_INT);

	// Load cubemap shader and setup required shader locations
	shdrCubemap = LoadShader((Knight::Instance->Config.ResourcesBasePath + KNIGHT_SHADER_DIRECTORY "cubemap.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + KNIGHT_SHADER_DIRECTORY "cubemap.fs").c_str());

	int val3[1] = { 0 };
	SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), val3, SHADER_UNIFORM_INT);

	Image img = LoadImage(pSkyboxTexFilePath);
	if (img.data == nullptr)
	{
		TraceLog(LOG_ERROR, "Skybox: failed to load image: %s", pSkyboxTexFilePath);
		return false;
	}

	skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, layout);
	UnloadImage(img);
	if (skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture.id == 0)
	{
		TraceLog(LOG_ERROR, "Skybox: failed to create cubemap: %s", pSkyboxTexFilePath);
		return false;
	}

	TraceLog(LOG_INFO, "Skybox: cubemap loaded: %s", pSkyboxTexFilePath);

	return true;
}

void SkyboxComponent::Draw(RenderHints* pRH)
{
	//We are inside the cube, we need to disable backface culling!
	rlDisableBackfaceCulling();
	rlDisableDepthMask();
	DrawModel(skybox, Vector3{ 0, 0, 0 }, 1, WHITE);
	rlEnableBackfaceCulling();
	rlEnableDepthMask();
}
