#include "ParticleComponent.h"

bool ParticleComponent::CreateFromFile(const char* path, int maxp, Vector3 v, Color ic, Vector3 isp)
{
    Texture2D particleTexture = { 0 };

    particleTexture = LoadTextureForPlatform(path);
    if (particleTexture.width == 0 && particleTexture.height == 0)
        return false;
    maxParticles = maxp;
    offset = v;
    texture = particleTexture;
    offset = v;
    initialColor = ic;
    initialSpeed = isp;
    particles.reserve(maxParticles);

    return true;
}

void ParticleComponent::Update(float ElapsedSeconds, RenderHints* pRH)
{
    for (auto& particle : particles) {
        // Update particle position based on velocity
        particle.position.x += particle.velocity.x * ElapsedSeconds;
        particle.position.y += particle.velocity.y * ElapsedSeconds;
        particle.position.z += particle.velocity.z * ElapsedSeconds;

        // Apply gravity
        particle.velocity.y -= 9.8f * ElapsedSeconds;

        // Decrease particle life
        particle.life -= ElapsedSeconds;
        float lifeRatio = particle.life / particle.maxLife;

        // Fade out as life decreases
        particle.color.a = static_cast<unsigned char>(255 * lifeRatio);
    }

    // Remove dead particles
    particles.erase(std::remove_if(particles.begin(), particles.end(),
        [](const Particle& p) { return p.life <= 0; }),
        particles.end());

    // Emit new particles
    EmitParticles(ElapsedSeconds);
}

void ParticleComponent::Draw(RenderHints* pRH)
{
    Camera3D *pCam = NULL;
    SceneCamera* pMainCam = _SceneActor->GetMainCamera();
    if (pMainCam != NULL)
        pCam = pMainCam->GetCamera3D();

    if (pCam != NULL) {
        for (const auto& particle : particles) {
            float size = 0.3f; // Size of each particle
            DrawBillboard(*pCam, texture, particle.position, size, particle.color);
        }
    }
}

void ParticleComponent::EmitParticles(float deltaTime)
{
    int particlesToEmit = 5; // Emit rate

    Vector3 origin = Vector3Add(_SceneActor->Position, offset);

    for (int i = 0; i < particlesToEmit && particles.size() < maxParticles; i++) {
        Particle particle;
        particle.position = origin;

        // Random velocity with some upward direction
        particle.velocity.x = (float(rand()) / RAND_MAX - 0.5f) * 2.0f;
        particle.velocity.y = (float(rand()) / RAND_MAX) * 2.0f + 2.0f; // Upward velocity
        particle.velocity.z = (float(rand()) / RAND_MAX - 0.5f) * 2.0f;
        Vector3Add(particle.velocity, initialSpeed);

        particle.life = particle.maxLife = 2.0f + float(rand()) / RAND_MAX * 2.0f; // Life in seconds
        particle.color = initialColor; // Initial color

        particles.push_back(particle);
    }
}

//End of ParticleSystem.cpp
