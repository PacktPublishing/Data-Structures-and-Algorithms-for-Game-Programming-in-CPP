//TerrainEntity class implementation.
#include <random>

#include "BonusGameWorld02.h"

bool TerrainEntity::Create(Scene* pScene, Entity* pContainer)
{
	_Actor = pScene->CreateSceneObject<SceneActor>("Terrain");
	_Terrain = _Actor->CreateAndAddComponent<QuadTreeTerrainComponent>();
	_Terrain->receiveShadow = true;
	_Terrain->castShadow = Component::eShadowCastingType::Shadow;
	bool success = _Terrain->CreateFromFile(Vector3{ 512,16,512 }, Vector2{ 1,1 }, (Knight::Instance->Config.ResourcesBasePath + "textures/heightmap.png").c_str(), (Knight::Instance->Config.ResourcesBasePath + "textures/terrain_map.png").c_str());	

	if (!success)
	{
		TraceLog(LOG_ERROR, "<TerrainEntity.Create> Failed to create terrain from file!");
		return false;
	}

	//Load a texture as billboard image
	treeImage = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/p15-2.png").c_str());    // Our tree billboard texture
	robotImage = LoadTexture((Knight::Instance->Config.ResourcesBasePath + "textures/p13.png").c_str());    // Our robot billboard texture
	// Create a random device and seed the Mersenne Twister engine
	random_device rd;
	mt19937 gen(rd());
	// Define a uniform real distribution in the range [-128, 128]
	uniform_real_distribution<float> randomTerrainRange(-128.0f, 128.0f);
	// Define a uniform real distribution in the range [1, 255]
	uniform_real_distribution<float> randomColorRange(1.0f, 255.0f);
	// Define a uniform real distribution in the range [4, 8]
	uniform_real_distribution<float> randomSizeRange(4.0f, 8.0f);

	//create billboard imposters on the terrain
	for (int i = 0; i < 600; i++)
	{
		//imposter (billboard)
		SceneActor* imposter = pScene->CreateSceneObject<SceneActor>(TextFormat("Prop%3d", i));
		imposter->Position = Vector3{ randomTerrainRange(gen),0, randomTerrainRange(gen) };
		imposter->Rotation = Vector3{ 0,0,0 };
		BillboardComponent* billboard = imposter->CreateAndAddComponent<BillboardComponent>();

		//initialize billbard
		if (randomTerrainRange(gen) > 16.0f) {
			billboard->texture = treeImage;
			billboard->receiveShadow = false;
		}
		else {
			billboard->texture = robotImage;
			billboard->receiveShadow = true;
		}
		// Entire billboard texture, source is used to take a segment from a larger texture.
		billboard->source = { 0.0f, 0.0f, (float)billboard->texture.width, (float)billboard->texture.height };
		billboard->size = { randomSizeRange(gen), randomSizeRange(gen) };
		billboard->origin = Vector2Scale(billboard->size, 0.5f);
		billboard->renderQueue = Component::eRenderQueueType::AlphaBlend;
		billboard->blendingMode = BLEND_ALPHA;
		billboard->EnableAlphaTest = true;
		billboard->castShadow = Component::eShadowCastingType::Shadow;

		//adjust tree height based on terrain
		imposter->Position.y = _Terrain->GetTerrainY(imposter->Position.x, imposter->Position.z) + billboard->size.y * 0.45f; // Set height based on terrain

		imposters.push_back(imposter);
	}

	for(int i = 0; i < 20; i++) {
		// Set up particle system
		SceneActor* pParticleActor = pScene->CreateSceneObject<SceneActor>(TextFormat("Particle%d", i));
		pParticleActor->Position = Vector3{ randomTerrainRange(gen),0, randomTerrainRange(gen) };
		pParticleActor->Position.y = _Terrain->GetTerrainY(pParticleActor->Position.x, pParticleActor->Position.z) + 0.5f; // Set height based on terrain
		ParticleComponent* pParticle = pParticleActor->CreateAndAddComponent<ParticleComponent>();
		pParticle->CreateFromFile((Knight::Instance->Config.ResourcesBasePath + "textures/flash00.png").c_str(), 300, Vector3{ 0,0,0 }, Color{ (unsigned char)randomTerrainRange(gen), (unsigned char)randomTerrainRange(gen) , (unsigned char)randomTerrainRange(gen), 255 }, Vector3{ 0,0,0 });
	}

	//create a skybox under main camera actor
	SceneCamera* pMainCamera = pScene->GetMainCameraActor();
	pSkybox = pMainCamera->CreateAndAddComponent<SkyboxComponent>();
	pSkybox->renderQueue = Component::eRenderQueueType::Background;
	pSkybox->castShadow = Component::eShadowCastingType::NoShadow;
	pSkybox->receiveShadow = false;
	pSkybox->CreateFromFile((Knight::Instance->Config.ResourcesBasePath + "textures/skybox2.png").c_str(), CUBEMAP_LAYOUT_CROSS_FOUR_BY_THREE, 5.0f, false);

	return true;
}

static Color dayToNightCycle[32] = {
	// Morning (Dawn to Early Morning)
	{255, 240, 230, 255}, // Dawn (light yellow-pink)
	{250, 220, 200, 255}, // Soft sunrise
	{240, 200, 170, 255}, // Early morning (warmer orange tones)
	{230, 180, 150, 255}, // Mid-morning
	{220, 200, 170, 255}, // Late morning (approaching noon)

	// Midday (Bright Daylight)
	{255, 255, 255, 255}, // Noon (bright white, peak of the day)
	{240, 245, 255, 255}, // Slightly dimmer
	{230, 240, 255, 255}, // Early afternoon (light blue sky)
	{220, 235, 255, 255}, // Softer blue sky
	{210, 225, 255, 255}, // Mid-afternoon

	// Late Afternoon
	{200, 210, 255, 255}, // Late afternoon, cooling down
	{190, 200, 245, 255}, // Early evening (light orange hint)
	{180, 190, 235, 255}, // Sunset starts
	{170, 175, 225, 255}, // More pronounced sunset
	{160, 165, 215, 255}, // Dusk (warm-to-cool transition)

	// Evening and Early Night
	{150, 155, 205, 255}, // End of dusk, transitioning to night
	{140, 145, 195, 255}, // Early night (soft blue tones)
	{130, 135, 185, 255}, // Darker night
	{120, 125, 175, 255}, // Deeper blue

	// Midnight
	{110, 115, 160, 255}, // Midnight (deep blue)
	{90, 95, 140, 255},   // Post-midnight (darker)
	{70, 75, 120, 255},   // Night approaching its darkest
	{50, 55, 100, 255},   // Darkest point of night

	// Pre-Dawn
	{40, 45, 90, 255},    // Pre-dawn (very dark blue)
	{30, 40, 80, 255},    // Starting to lighten up
	{50, 55, 100, 255},   // Hints of color returning (blue)
	{70, 75, 120, 255},   // Gradual lightening
	{90, 95, 140, 255},   // Early dawn

	// Returning to Daylight (Dawn)
	{120, 125, 175, 255}, // Gradually lightening sky
	{150, 155, 205, 255}, // Colors warming up (early morning)
	{190, 200, 245, 255}, // Early morning light (back to day)
	{230, 240, 255, 255}//, // Soft blue sky (late morning)
	//{255, 255, 255}  // Midday again (completes the cycle)
};

void TerrainEntity::Update(float ElapsedSeconds)
{
	timeDiff += ElapsedSeconds;

	if (timeDiff > 1.0f) {
		currentIdx++;
		timeDiff = 0.0f;
	}
	if (currentIdx >= 32)
		currentIdx -= 32;

	int nextIdx;
	Color c1 = dayToNightCycle[currentIdx];
	if (currentIdx < 31)
		nextIdx = currentIdx + 1;
	else
		nextIdx = 0;
	Color c2 = dayToNightCycle[nextIdx];

	float t = timeDiff;

	pSkybox->_SkyColor = WHITE;
	pSkybox->_SkyColor.r = (unsigned char)((1 - t) * c1.r + t * c2.r);
	pSkybox->_SkyColor.g = (unsigned char)((1 - t) * c1.g + t * c2.g);
	pSkybox->_SkyColor.b = (unsigned char)((1 - t) * c1.b + t * c2.b);

}