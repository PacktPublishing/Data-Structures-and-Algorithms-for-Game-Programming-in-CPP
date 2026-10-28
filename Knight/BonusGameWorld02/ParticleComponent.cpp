//This is the same ParticleComponent.cpp file from Chapter 07 sample project, slightly modified.
#include "ParticleComponent.h"

ParticleComponent::ParticleComponent()
{
    renderQueue = Component::eRenderQueueType::AlphaBlend;
    blendingMode = BLEND_ADDITIVE;
    EnableAlphaTest = false;
}

bool ParticleComponent::CreateFromFile(const char* path, int maxp, Vector3 v, Color ic, Vector3 isp)
{
    Texture2D particleTexture = { 0 };

    particleTexture = LoadTextureForPlatform(path);
    if (particleTexture.width == 0 && particleTexture.height == 0)
        return false;
    maxParticles = maxp;
    offset = v;
    texture = particleTexture;
    initialColor = ic;
    initialSpeed = isp;
    particles.reserve(maxParticles);

	return true;
}

//Animtae existing particles, remove dead ones, emit new ones
void ParticleComponent::Update(float ElapsedSeconds, RenderHints* pRH)
{
    if (currentDelayTime > 0.0f) {
        currentDelayTime -= ElapsedSeconds;
        return; // Wait until delay is over
	}

    for (auto& particle : particles) {
        // Update particle position based on velocity
        particle.position.x += particle.velocity.x * ElapsedSeconds;
        particle.position.y += particle.velocity.y * ElapsedSeconds;
        particle.position.z += particle.velocity.z * ElapsedSeconds;

		// Apply small gravity to make it fly up 
        particle.velocity.y += 0.8f * ElapsedSeconds;

        // Decrease particle life
        particle.life -= ElapsedSeconds;
        float lifeRatio = particle.life / particle.maxLife;

        particle.scale = 1.0f;

        // Fade out as life decreases
        particle.color.a = static_cast<unsigned char>(255 * lifeRatio);
		particle.color.r = static_cast<unsigned char>(initialColor.r * lifeRatio);
        particle.color.g = static_cast<unsigned char>(initialColor.g * lifeRatio);
        particle.color.b = static_cast<unsigned char>(initialColor.b * lifeRatio);
    }

    // Remove dead particles
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.life <= 0; }),
        particles.end());

    // Emit new particles
    EmitParticles(ElapsedSeconds);
}

// Draw all particles as billboards
void ParticleComponent::Draw(RenderHints* pRH)
{
    Camera3D *pCam = NULL;
    SceneCamera* pMainCam = _SceneActor->GetMainCamera();
    if (pMainCam != NULL)
        pCam = pMainCam->GetCamera3D();

    if (pCam != NULL) {
        for (const auto& particle : particles) {
            Vector3 worldPos = Vector3Transform(particle.position, *_SceneActor->GetWorldTransformMatrix());
            DrawBillboard(*pCam, texture, worldPos, particle.scale, particle.color);
        }
    }
}

// Emit new particles
void ParticleComponent::EmitParticles(float deltaTime)
{
    int particlesToEmit = 5; // Emit rate

    for (int i = 0; i < particlesToEmit && particles.size() < maxParticles; i++) {
        Particle particle;
        particle.position = offset;
        particle.scale = 1.0f;

        // Random velocity with some upward direction
        particle.velocity.x = (float(rand()) / RAND_MAX - 0.5f) * 2.0f;
        particle.velocity.y = (float(rand()) / RAND_MAX) * 2.0f + 4.0f; // Upward velocity
        particle.velocity.z = (float(rand()) / RAND_MAX - 0.5f) * 2.0f;
        Vector3Add(particle.velocity, initialSpeed);

        particle.life = particle.maxLife = 2.0f + float(rand()) / RAND_MAX * 2.0f; // Life in seconds
        particle.color = initialColor; // Initial color

        particles.push_back(particle);
    }
}

//End of ParticleSystem.cpp
