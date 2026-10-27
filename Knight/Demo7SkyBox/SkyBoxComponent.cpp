#include "SkyboxComponent.h"
#include "Knight.h"

SkyboxComponent::SkyboxComponent()
{
	// Load skybox model
	cube = GenInwardCube(1.0f);
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

	// Load skybox shader and set required locations
	// NOTE: Some locations are automatically set at shader loading
	skybox.materials[0].shader = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/skybox.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/skybox.fs").c_str());

	int val1[1] = { MATERIAL_MAP_CUBEMAP };
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), val1, SHADER_UNIFORM_INT);
	int val2[1] = { 0 };
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "doGamma"), val2, SHADER_UNIFORM_INT);
	SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "vflipped"), val2, SHADER_UNIFORM_INT);

	// Load cubemap shader and setup required shader locations
	shdrCubemap = LoadShader((Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/cubemap.vs").c_str(), (Knight::Instance->Config.ResourcesBasePath + "shaders/glsl330/cubemap.fs").c_str());

	int val3[1] = { 0 };
	SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), val3, SHADER_UNIFORM_INT);

	Image img = LoadImage(pSkyboxTexFilePath);
	skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = LoadTextureCubemap(img, layout);    
	UnloadImage(img);

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

Mesh SkyboxComponent::GenInwardCube(float size) 
{
    Mesh mesh = { 0 };

    // Define the positions of the cube vertices
    float halfSize = size / 2.0f;

    Vector3 positions[] = {
        // Front face
        { -halfSize, -halfSize,  halfSize },
        {  halfSize, -halfSize,  halfSize },
        {  halfSize,  halfSize,  halfSize },
        { -halfSize,  halfSize,  halfSize },

        // Back face
        { -halfSize, -halfSize, -halfSize },
        { -halfSize,  halfSize, -halfSize },
        {  halfSize,  halfSize, -halfSize },
        {  halfSize, -halfSize, -halfSize },

        // Left face
        { -halfSize, -halfSize, -halfSize },
        { -halfSize, -halfSize,  halfSize },
        { -halfSize,  halfSize,  halfSize },
        { -halfSize,  halfSize, -halfSize },

        // Right face
        { halfSize, -halfSize, -halfSize },
        { halfSize,  halfSize, -halfSize },
        { halfSize,  halfSize,  halfSize },
        { halfSize, -halfSize,  halfSize },

        // Top face
        { -halfSize,  halfSize, -halfSize },
        { -halfSize,  halfSize,  halfSize },
        {  halfSize,  halfSize,  halfSize },
        {  halfSize,  halfSize, -halfSize },

        // Bottom face
        { -halfSize, -halfSize, -halfSize },
        {  halfSize, -halfSize, -halfSize },
        {  halfSize, -halfSize,  halfSize },
        { -halfSize, -halfSize,  halfSize },
    };

    // Define the normals for each face pointing inward
    Vector3 normals[] = {
        // Front face normals
        {  0.0f,  0.0f, -1.0f }, {  0.0f,  0.0f, -1.0f },
        {  0.0f,  0.0f, -1.0f }, {  0.0f,  0.0f, -1.0f },

        // Back face normals
        {  0.0f,  0.0f,  1.0f }, {  0.0f,  0.0f,  1.0f },
        {  0.0f,  0.0f,  1.0f }, {  0.0f,  0.0f,  1.0f },

        // Left face normals
        {  1.0f,  0.0f,  0.0f }, {  1.0f,  0.0f,  0.0f },
        {  1.0f,  0.0f,  0.0f }, {  1.0f,  0.0f,  0.0f },

        // Right face normals
        { -1.0f,  0.0f,  0.0f }, { -1.0f,  0.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f }, { -1.0f,  0.0f,  0.0f },

        // Top face normals
        {  0.0f, -1.0f,  0.0f }, {  0.0f, -1.0f,  0.0f },
        {  0.0f, -1.0f,  0.0f }, {  0.0f, -1.0f,  0.0f },

        // Bottom face normals
        {  0.0f,  1.0f,  0.0f }, {  0.0f,  1.0f,  0.0f },
        {  0.0f,  1.0f,  0.0f }, {  0.0f,  1.0f,  0.0f },
    };

    // Define the indices for the cube (two triangles per face)
    unsigned short indices[] = {
        0, 1, 2,  2, 3, 0,  // Front face
        4, 5, 6,  6, 7, 4,  // Back face
        8, 9, 10, 10, 11, 8, // Left face
        12, 13, 14, 14, 15, 12, // Right face
        16, 17, 18, 18, 19, 16, // Top face
        20, 21, 22, 22, 23, 20  // Bottom face
    };

    // Assign mesh data
    mesh.vertexCount = sizeof(positions) / sizeof(Vector3);
    mesh.triangleCount = sizeof(indices) / (3 * sizeof(unsigned short));
    mesh.vertices = (float*)positions;
    mesh.normals = (float*)normals;
    mesh.indices = indices;

    // Upload the mesh to the GPU
    UploadMesh(&mesh, true);

    return mesh;
}