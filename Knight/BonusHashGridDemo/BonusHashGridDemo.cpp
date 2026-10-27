#include "BonusHashGridDemo.h"

//main entry point for the application
int main(int argc, char* argv[])
{
    BonusHashGridDemo* KnightHashGridDemo = new BonusHashGridDemo();

    KnightHashGridDemo->Start();
    KnightHashGridDemo->GameLoop();

    delete KnightHashGridDemo;
    return 0;
}

void BonusHashGridDemo::Start()
{
	__super::Start();

    // Entities
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> rx(-HALF + 1.0f, HALF - 1.0f);
    std::uniform_real_distribution<float> rz(-HALF + 1.0f, HALF - 1.0f);
    std::uniform_real_distribution<float> rv(-7.0f, 7.0f);
    std::uniform_real_distribution<float> rh(0.35f, 0.8f);

	ents.resize(NUM);

    for (size_t i = 0; i < ents.size(); ++i) {
        ents[i].pos = Vector3{ rx(rng), 0.0f, rz(rng) };
        ents[i].vel = Vector3{ rv(rng), 0.0f, rv(rng) };
        ents[i].half = rh(rng);
        ents[i].pos.y = ents[i].half; // sit on ground
        ents[i].base = Color{ 190, 210, 235, 255 };
        ents[i].highlight = false;
        //ents[i].hit = false;
    }

	// Sparse hash: Cell -> indices
    grid.reserve(NUM * 2);

    transforms.resize(ents.size());

    //Prepare a built-in camera and set its properties
    camera = _Scene->CreateSceneObject<FlyThroughCamera>("Camera");
	camera->SetUp(Vector3{ 0,0,0 }, 30.0f, 45.0f, 25.0f, 60.0f, CAMERA_PERSPECTIVE);
    camera->ShowCursor = false;
}

void BonusHashGridDemo::Update(float ElapsedSeconds)
{
    __super::Update(ElapsedSeconds);

    // Adjust area radius 
    if (IsKeyPressed(KEY_X)) 
    {
        areaRadius += 3.0f;
        EnsureRadiusRange();
    }

    if (IsKeyPressed(KEY_Z))
    {
        areaRadius -= 3.0f;
        EnsureRadiusRange();
    }

    // Simulate + reset flags
    for (size_t i = 0; i < ents.size(); ++i) {

        CubeEnt& e = ents[i];

		//move to new position based on velocity
        e.pos.x += e.vel.x * DT;
        e.pos.z += e.vel.z * DT;

        if (e.pos.x < -HALF + e.half) { e.pos.x = -HALF + e.half; e.vel.x *= -1.0f; }
        if (e.pos.x > HALF - e.half) { e.pos.x = HALF - e.half; e.vel.x *= -1.0f; }
        if (e.pos.z < -HALF + e.half) { e.pos.z = -HALF + e.half; e.vel.z *= -1.0f; }
        if (e.pos.z > HALF - e.half) { e.pos.z = HALF - e.half; e.vel.z *= -1.0f; }

        e.pos.y = e.half;
        e.highlight = false;
    }

    // Rebuild sparse hash
    grid.clear();
    for (int i = 0; i < (int)ents.size(); ++i) {
        Cell c = CellForXZ(ents[i].pos, CELL);
        grid[c].push_back(i);
    }

    // Area center from MMB
    if (IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
        Ray ray = GetMouseRay(GetMousePosition(), *camera->GetCamera3D());
        if (std::fabs(ray.direction.y) > 1e-6f) {
            float t = -ray.position.y / ray.direction.y; // y=0
            if (t > 0.0f) {
                Vector3 p = Vector3Add(ray.position, Vector3Scale(ray.direction, t));
                areaCenter = Vector3{ p.x, 0.0f, p.z };
                areaActive = true;
            }
        }
    }

    if (IsKeyPressed(KEY_Q)) 
        areaActive = !areaActive;

    // Ray casting using grid-DDA on XZ, hash lookup per visited cell, precise ray-AABB
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Ray ray = GetMouseRay(GetMousePosition(), *camera->GetCamera3D());

        float tStart = 0.0f;
        if (std::fabs(ray.direction.y) > 1e-6f) {
            float tPlane = -ray.position.y / ray.direction.y;
            if (tPlane > 0.0f) tStart = tPlane;
        }

        const float tEnd = 250.0f;
        DDA2D dda = InitDDA_XZ(ray, tStart, CELL);

        float bestT = tEnd + 1.0f;
        int   bestIdx = -1;

        for (int steps = 0; steps < 3000; ++steps) {
            if (dda.t > tEnd) break;

            // hash lookup for current cell
            std::unordered_map<Cell, std::vector<int>, CellHash>::const_iterator it =
                grid.find(Cell{ dda.ix, dda.iz });
            if (it != grid.end()) {
                const std::vector<int>& bucket = it->second;
                for (size_t k = 0; k < bucket.size(); ++k) {
                    int idx = bucket[k];
                    float tHit;
                    if (RayAABB(ray, ents[idx].min(), ents[idx].max(), tHit)) {
                        if (tHit < bestT) { bestT = tHit; bestIdx = idx; }
                    }
                }
                if (bestIdx >= 0 && bestT <= dda.t) break; // hit before next boundary
            }

            DDA_Step_XZ(dda);
        }

        //if (bestIdx >= 0) ents[bestIdx].hit = true;
        if (bestIdx >= 0) {
            selectIdx = bestIdx;
        }
        else
        {
            selectIdx = -1;
        }
    }

	//If any cube is selected, activate area query and center it
    if (selectIdx >= 0) 
    {
        if (areaActive == false)
            areaActive = true;
        areaCenter = ents[selectIdx].pos;
    }

    // Area query via hash (circle in XZ vs each cube AABB)
    if (areaActive) {
        int minCx = (int)std::floor((areaCenter.x - areaRadius) / CELL);
        int maxCx = (int)std::floor((areaCenter.x + areaRadius) / CELL);
        int minCz = (int)std::floor((areaCenter.z - areaRadius) / CELL);
        int maxCz = (int)std::floor((areaCenter.z + areaRadius) / CELL);

        for (int cx = minCx; cx <= maxCx; ++cx) {
            for (int cz = minCz; cz <= maxCz; ++cz) {
                std::unordered_map<Cell, std::vector<int>, CellHash>::const_iterator it = grid.find(Cell{ cx, cz });
                if (it == grid.end()) continue;
                const std::vector<int>& bucket = it->second;

                for (size_t k = 0; k < bucket.size(); ++k) {
                    int idx = bucket[k];
                    if (CircleIntersectsAABB_XZ(areaCenter, areaRadius, ents[idx].min(), ents[idx].max()))
                        ents[idx].highlight = true;
                }
            }
        }
    }

}

void BonusHashGridDemo::DrawFrame()
{
	__super::DrawFrame();

	// Draw ground plane and grid
    DrawPlane(Vector3{ 0, 0, 0 }, Vector2{ HALF * 2.0f, HALF * 2.0f }, Color{ 28, 34, 42, 255 });
    Color gridC = Color{ 54,60,70,255 };
    for (float x = -HALF; x <= HALF; x += CELL) DrawLine3D(Vector3{ x, 0, -HALF }, Vector3{ x, 0, HALF }, gridC);
    for (float z = -HALF; z <= HALF; z += CELL) DrawLine3D(Vector3{ -HALF, 0, z }, Vector3{ HALF, 0, z }, gridC);

	//Show a nice shper to visualize the range of area query
    if (areaActive) {
		DrawSphereWires(areaCenter, areaRadius, 16, 16, Color{ 200, 180, 50, 255 });
    }

	// Rendering cubes
    for (size_t i = 0; i < ents.size(); ++i) {
        Color c = ents[i].base;
        if (ents[i].highlight) c = ORANGE;
        if (i == selectIdx) c = GREEN;
        // Draw cube using size = 2*half
        DrawCube(ents[i].pos, 2.0f * ents[i].half, 2.0f * ents[i].half, 2.0f * ents[i].half, c);
        DrawCubeWires(ents[i].pos, 2.0f * ents[i].half, 2.0f * ents[i].half, 2.0f * ents[i].half, 
            Color{
            80, 90, 100, 255
            });
    }
}

void BonusHashGridDemo::DrawGUI()
{
    DrawText("Ray-AABB + Spatial Hash + Grid-DDA (XZ) Demo", 16, 50, 40, RAYWHITE);
    DrawText("- Left mouse click to perform ray picking via DDA+spatical hash", 16, 100, 40, RAYWHITE);
    DrawText("- Use arrow keys to move camera, hold mouse right button to rotate.", 16, 150, 40, RAYWHITE);
    DrawText("- Use Z, X key to change active area radius, Q: toggle area", 16, 200, 40, RAYWHITE);
}

// This function is called when the application is created.
// It is used to load default resources such as fonts.
void BonusHashGridDemo::OnCreateDefaultResources()
{
    __super::OnCreateDefaultResources();
    //Loads a better TrueType font to display text information on the screen
    _Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
	Config.ShowFPS = true;
}
