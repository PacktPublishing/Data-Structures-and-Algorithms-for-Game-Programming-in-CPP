#pragma once

#include <vector>

#include "Knight.h"

// ----------------------------- Config ----------------------------------
static constexpr int   TOTAL_CUBES = 400;     // change as you like
static constexpr float CUBE_SIZE = 1.2f;
static constexpr float MIN_SPACING = 2.5f * CUBE_SIZE;   // hard no-overlap
static constexpr float AREA_HALF = 40.0f;              // half-extent of placement area
static constexpr float NEIGHBOR_R = 6.0f;    // radius for "neighbors"

#include "KdTree2XZ.h"

struct Cube 
{ 
	Vector3 pos; 
	Color color; 
};

class BonusKdTreeDemo : public Knight
{
	public:
		void Start() override;
		void EndGame() override;

	protected:
		void Update(float ElapsedSeconds) override;
		void DrawFrame() override;
		void DrawGUI() override;
		void OnCreateDefaultResources() override;
		void OnConfigureInput() override;

		void GenerateCubesNoOverlap(vector<Cube>& cubes, vector<Vector3>& positions, KdTree2XZ& tree);

		vector<Cube> cubes;
		vector<Vector3> positions;
		KdTree2XZ tree;

		int selectedIdx = -1;  // remember last clicked cube

		InputAction regenerateAction;
};

//End of file BonusKdTreeDemo.h