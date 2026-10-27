#include "Demo4TexOps.h"

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

ColorBlendingDemo::ColorBlendingDemo()
{
	title = "Color Blending Demo";
	description = "This demo loads a png graphics file as texture and draw it with color blending\n\n\nto simulate lighting changes from night to day cycle.";
    timeDiff = 0;
    currentIdx = 0;
}

void ColorBlendingDemo::Create()
{
	Image art = LoadImage((Knight::Instance->Config.ResourcesBasePath + "textures/lake.png").c_str());
	texture = LoadTextureFromImage(art);
	UnloadImage(art);
	isReady = true;
}

void ColorBlendingDemo::Update(float elapsed)
{
	timeDiff += elapsed;

    if (timeDiff > 1.0f) {
        currentIdx++;
        timeDiff = 0.0f;
    }
    if (currentIdx >= 32)
        currentIdx -= 32;
}

void ColorBlendingDemo::Draw2D()
{
	Vector2 position = { 0,0 };
    DrawTextureEx(texture, position, 0, 1.5f, dayToNightCycle[currentIdx]);
}

void ColorBlendingDemo::Release()
{
	UnloadTexture(texture);
	isReady = false;
}

SmoothColorBlendingDemo::SmoothColorBlendingDemo()
{
    title = "Smooth Color Blending Demo";
    description = "This demo enhances previous demo with smooth interpolation between two color changes.";
}

void SmoothColorBlendingDemo::Draw2D()
{
    Vector2 position = { (float)(SCREEN_WIDTH - texture.width) / 2.0f, (float)(SCREEN_HEIGHT - texture.height) / 2.0f };
    int nextIdx;
    Color c1 = dayToNightCycle[currentIdx];
    if (currentIdx < 31)
        nextIdx = currentIdx + 1;
    else
        nextIdx = 0;
    Color c2 = dayToNightCycle[nextIdx];

    float t = timeDiff;

    Color c = WHITE;
    c.r = (unsigned char)((1 - t)* c1.r + t * c2.r);
    c.g = (unsigned char)((1 - t) * c1.g + t * c2.g);
    c.b = (unsigned char)((1 - t) * c1.b + t * c2.b);

    DrawTexture(texture, (int)position.x, (int)position.y, c);
}
