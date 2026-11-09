#pragma once

#include "Knight.h"

class Demo3d : public Knight
{
public:
	void Start() override;

protected:
	void Update(float ElapsedSeconds) override;
	void DrawGUI() override;
	void EndGame() override;
	void OnConfigureInput() override;

private:
	void Shuffle();
	void SortDealtCards();
};

