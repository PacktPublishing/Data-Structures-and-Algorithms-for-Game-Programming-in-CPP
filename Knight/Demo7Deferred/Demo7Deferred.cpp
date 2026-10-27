#include "raylib.h"
#include "rlgl.h"
#include "Defs.h"
#include <string>

RenderTexture2D LoadRenderTextureEx(int width, int height);

int main() {
    const int screenWidth = 1920;
    const int screenHeight = 1080;
    InitWindow(screenWidth, screenHeight, "Deferred Rendering Example");

    // Camera setup
    Camera3D camera = { 0 };
    camera.position = { 5.0f, 5.0f, -5.0f };
    camera.target = { 0.0f, 1.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    // Create G-buffer
    RenderTexture2D gPosition = LoadRenderTextureEx(screenWidth, screenHeight);
    RenderTexture2D gNormal = LoadRenderTextureEx(screenWidth, screenHeight);
    RenderTexture2D gAlbedo = LoadRenderTextureEx(screenWidth, screenHeight);

    // Load shaders
    const std::string resourcesBasePath = RESOURCE_ROOT_PATH;

    // Load shaders
    Shader geometryShader = LoadShader((resourcesBasePath + "shaders/glsl330/def_geometry.vs").c_str(), (resourcesBasePath + "shaders/glsl330/def_geometry.fs").c_str());
    Shader lightingShader = LoadShader((resourcesBasePath + "shaders/glsl330/def_lighting.vs").c_str(), (resourcesBasePath + "shaders/glsl330/def_lighting.fs").c_str());

    // Configure lighting shader uniforms
    Vector3 lightPosition = { 2.0f, 4.0f, 2.0f };
    Vector3 lightColor = { 1.0f, 1.0f, 1.0f };
    Vector3 viewPosition = camera.position;

    int locLightPosition = GetShaderLocation(lightingShader, "lightPosition");
    int locLightColor = GetShaderLocation(lightingShader, "lightColor");
    int locViewPosition = GetShaderLocation(lightingShader, "viewPosition");

    int texPos = GetShaderLocation(lightingShader, "gPosition");
    int texNormal = GetShaderLocation(lightingShader, "gNormal");
    int texAbedo = GetShaderLocation(lightingShader, "gAlbedo");

    SetShaderValue(lightingShader, locLightPosition, &lightPosition, SHADER_UNIFORM_VEC3);
    SetShaderValue(lightingShader, locLightColor, &lightColor, SHADER_UNIFORM_VEC3);
    SetShaderValue(lightingShader, locViewPosition, &viewPosition, SHADER_UNIFORM_VEC3);

    //SetShaderValue(lightingShader, texPos, &gPosition.texture, SHADER_UNIFORM_SAMPLER2D);
    //SetShaderValue(lightingShader, texNormal, &gNormal.texture, SHADER_UNIFORM_SAMPLER2D);
    //SetShaderValue(lightingShader, texAbedo, &gAlbedo.texture, SHADER_UNIFORM_SAMPLER2D);

    // Create scene objects
    Model model = LoadModelFromMesh(GenMeshCube(1.0f, 1.0f, 1.0f));
    Texture2D texture = LoadTexture((resourcesBasePath + "models/obj/wall_diffuse.png").c_str());
    model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = texture;

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        UpdateCamera(&camera, CAMERA_CUSTOM);

        //Geometry Pass : Render to G - buffer
        BeginTextureMode(gPosition);
        BeginMode3D(camera);
        ClearBackground(BLUE);
        //model.materials[0].shader = geometryShader;
        DrawModel(model, Vector3 { 0.0f, 1.0f, 0.0f }, 1.0f, WHITE);
        EndMode3D();
        EndTextureMode();

        /*BeginTextureMode(gNormal);
        BeginMode3D(camera);
        ClearBackground(BLANK);
        DrawModel(model, Vector3 { 0.0f, 1.0f, 0.0f }, 1.0f, WHITE);
        EndMode3D();
        EndTextureMode();

        BeginTextureMode(gAlbedo);
        BeginMode3D(camera);
        ClearBackground(BLANK);
        DrawModel(model, Vector3 { 0.0f, 1.0f, 0.0f }, 1.0f, WHITE);
        EndMode3D();
        EndTextureMode();*/

        // Lighting Pass: Render scene using G-buffer
        BeginDrawing();
        ClearBackground(BLACK);

 

        /*BeginMode3D(camera);
        ClearBackground(BLANK);
        DrawModel(model, Vector3{ 0.0f, 1.0f, 0.0f }, 1.0f, WHITE);
        EndMode3D();*/
        //DrawTextureEx(gPosition.texture, Vector2{ 0,0 }, 0, 1.0f, WHITE);

        //BeginShaderMode(lightingShader);
        //DrawTexture(gPosition.texture, 0, 0, WHITE);
        DrawTextureEx(gPosition.texture, Vector2{ 0,0 }, 0, 1.0f, WHITE);
        //EndShaderMode();

        EndDrawing();
    }

    // Cleanup
    UnloadRenderTexture(gPosition);
    UnloadRenderTexture(gNormal);
    UnloadRenderTexture(gAlbedo);
    UnloadShader(geometryShader);
    UnloadShader(lightingShader);
    UnloadModel(model);
    CloseWindow();

    return 0;
}

RenderTexture2D LoadRenderTextureEx(int width, int height)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, PIXELFORMAT_UNCOMPRESSED_R32G32B32A32, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = PIXELFORMAT_UNCOMPRESSED_R32G32B32A32;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}
