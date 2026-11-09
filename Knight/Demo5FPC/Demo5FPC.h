#pragma once

#include "Knight.h"   //Engine includes

#define MAX_COLUMNS 20

class Demo5FPC : public Knight
{
	public:
		void Start() override;

	protected:

		void DrawGUI() override;
		void OnCreateDefaultResources() override;
		void OnConfigureInput() override;
};


