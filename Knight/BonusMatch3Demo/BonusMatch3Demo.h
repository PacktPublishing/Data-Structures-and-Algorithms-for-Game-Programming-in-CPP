#pragma once

#include "Knight.h"

#include "Match3GameConfig.h"

#include "Match3GameSession.h"

class BonusMatch3Demo : public Knight
{
public:
	void Start() override;

protected:
	void Update(float ElapsedSeconds) override;
	void DrawFrame() override;

protected:
	Match3GameSession g;

private:
	void OnCreateDefaultResources() override;

};

//End of BonusMatch3Demo.h