
#include <random>

#include "BonusKdTreeDemo.h"

FlyThroughCamera* camera;

int main(int argc, char* argv[])
{
	BonusKdTreeDemo* KnightDemo = new BonusKdTreeDemo();

	KnightDemo->Start();
	KnightDemo->GameLoop();

	delete KnightDemo;
	return 0;
}

void BonusKdTreeDemo::Start()
{
	__super::Start();

	Config.ShowFPS = true;

	camera = _Scene->CreateSceneObject<FlyThroughCamera>("Camera");
	camera->SetUp(Vector3{ 0, 0, 0 }, 10.0f, 0, 35, 60, CAMERA_PERSPECTIVE);

	GenerateCubesNoOverlap(cubes, positions, tree);
}

void BonusKdTreeDemo::EndGame()
{
	__super::EndGame();
}

BoundingBox getCubeBBox(const Vector3& p)
{
	float h = CUBE_SIZE * 0.5f;
	return BoundingBox{ {p.x - h, -h, p.z - h}, {p.x + h, h, p.z + h} };
}

void BonusKdTreeDemo::Update(float ElapsedSeconds)
{
	if (IsKeyPressed(KEY_R))
	{
		GenerateCubesNoOverlap(cubes, positions, tree);
	}

	// Click detection
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
	{
		Ray ray = GetScreenToWorldRay(GetMousePosition(), *camera->GetCamera3D());

		int bestIdx = -1;
		float bestDist = 1e30f;
		for (int i = 0; i < (int)cubes.size(); ++i) 
		{
			RayCollision hit = GetRayCollisionBox(ray, getCubeBBox(cubes[i].pos));
			if (hit.hit) 
			{
				float d2 = Dist2XZ(ray.position, cubes[i].pos);
				if (d2 < bestDist) 
				{ 
					bestDist = d2; 
					bestIdx = i; 
				}
			}
		}

		// Reset all to WHITE
		for (auto& c : cubes) 
			c.color = GREEN;

		selectedIdx = bestIdx;

		if (bestIdx != -1) 
		{
			cubes[selectedIdx].color = RED; // highlight clicked cube

			// Find neighbors within NEIGHBOR_R and turn them BLUE
			std::vector<int> neighbors;
			tree.RadiusSearchXZ(cubes[selectedIdx].pos, NEIGHBOR_R * NEIGHBOR_R, neighbors);
			for (int idx : neighbors) 
			{
				if (idx == selectedIdx) 
					continue;
				cubes[idx].color = BLUE;
			}
		}
	}

	__super::Update(ElapsedSeconds);
}

void BonusKdTreeDemo::DrawFrame()
{
	DrawPlane({ 0,0,0 }, { AREA_HALF * 2.0f, AREA_HALF * 2.0f }, Color{ 235,235,235,255 });

	for (const auto& c : cubes) 
	{
		DrawCubeV(c.pos, { CUBE_SIZE, CUBE_SIZE, CUBE_SIZE }, c.color);
		DrawCubeWiresV(c.pos, { CUBE_SIZE, CUBE_SIZE, CUBE_SIZE }, BLACK);
	}

	// ---- Visual neighbor radius circle (on ground, around clicked cube) ----
	if (selectedIdx != -1) 
	{
		BeginBlendMode(BLEND_ADDITIVE);
		// A thin outline circle on the XZ plane. Normal=(0,1,0), angle=90 to lay flat.
		DrawSphere(cubes[selectedIdx].pos, NEIGHBOR_R, ColorAlpha(PURPLE, 0.55f));
		EndBlendMode();
	}

	__super::DrawFrame();
}

void BonusKdTreeDemo::DrawGUI()
{
	DrawRectangle(0, 0, GetScreenWidth(), 250, Color{ 0, 0, 0, 150 });

	DrawText("Use K-d tree to query neighbor cubes, ensure they are placed with minimum spacing.", 10, 50, 40, ORANGE);
	DrawText("Click on a cube to highlight it (red) and use K-d tree to find its neighbors (blue).", 10, 100, 40, ORANGE);
	DrawText("Move camera by arrow keys.", 10, 150, 40, ORANGE);
	DrawText("Rotate camera by right click and drag mouse. Mouse wheel to zoom.", 10, 200, 40, ORANGE);
}

void BonusKdTreeDemo::OnCreateDefaultResources()
{
	__super::OnCreateDefaultResources();
	_Font = LoadFontEx((Config.ResourcesBasePath + "fonts/sparky.ttf").c_str(), 32, 0, 0);
}

// ------------------------ Placement using kd-tree ----------------------
// Place N cubes at Y=0 with a hard minimum spacing using kd-tree radius checks.
void BonusKdTreeDemo::GenerateCubesNoOverlap(std::vector<Cube>& cubes, vector<Vector3>& positions, KdTree2XZ& tree) 
{
	cubes.clear();
	positions.clear();
	cubes.reserve(TOTAL_CUBES);
	positions.reserve(TOTAL_CUBES);

	std::mt19937 rng{ std::random_device{}() };
	std::uniform_real_distribution<float> d(-AREA_HALF, AREA_HALF);

	const float minR2 = MIN_SPACING * MIN_SPACING;

	for (int i = 0; i < TOTAL_CUBES; ++i) 
	{
		bool placed = false;
		for (int tries = 0; tries < 2000 && !placed; ++tries) 
		{
			Vector3 candidate{ d(rng), 0.0f, d(rng) };

			std::vector<int> neigh;
			tree.RadiusSearchXZ(candidate, (MIN_SPACING + CUBE_SIZE) * (MIN_SPACING + CUBE_SIZE), neigh);

			bool ok = true;
			for (int idx : neigh) 
			{
				if (Dist2XZ(candidate, positions[idx]) < minR2) 
				{ 
					ok = false; 
					break; 
				}
			}
			if (!ok) 
				continue;

			cubes.push_back({ candidate, GREEN });
			positions.push_back(candidate);
			tree.Build(positions);
			placed = true;
		}
	}
}


//End of file BonusKdTreeDemo.cpp
