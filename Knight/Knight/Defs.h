#pragma once

#define DEFAULT_APP_TITLE "Knight Demo"

#define DEFAULT_SCREEN_WIDTH  1920	
#define DEFAULT_SCREEN_HEIGHT 1080 

#define SCREEN_WIDTH  (Knight::Instance != nullptr ? Knight::Instance->Config.WindowWidth : DEFAULT_SCREEN_WIDTH)
#define SCREEN_HEIGHT (Knight::Instance != nullptr ? Knight::Instance->Config.WindowHeight : DEFAULT_SCREEN_HEIGHT)

#define TARGET_FPS 60

#define RESOURCE_ROOT_PATH "../../resources/"

#define MAX_SCENE_OBJECT_NAME	32
#define SCENE_ROOT_NAME "Scene Root"

//end of Defs.h