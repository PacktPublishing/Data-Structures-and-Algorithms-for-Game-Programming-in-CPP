#pragma once

#define DEFAULT_APP_TITLE "Knight Demo"

#define DEFAULT_SCREEN_WIDTH  1920	
#define DEFAULT_SCREEN_HEIGHT 1080 

#define SCREEN_WIDTH  (Knight::Instance != nullptr ? Knight::Instance->Config.WindowWidth : DEFAULT_SCREEN_WIDTH)
#define SCREEN_HEIGHT (Knight::Instance != nullptr ? Knight::Instance->Config.WindowHeight : DEFAULT_SCREEN_HEIGHT)

#define TARGET_FPS 60

#ifndef KNIGHT_RESOURCE_ROOT_PATH
#define KNIGHT_RESOURCE_ROOT_PATH "../../resources/"
#endif
#define RESOURCE_ROOT_PATH KNIGHT_RESOURCE_ROOT_PATH

#if defined(PLATFORM_WEB) || defined(PLATFORM_ANDROID)
#define KNIGHT_SHADER_DIRECTORY "shaders/glsl100/"
#define KNIGHT_GLSL_VERSION 100
#else
#define KNIGHT_SHADER_DIRECTORY "shaders/glsl330/"
#define KNIGHT_GLSL_VERSION 330
#endif

#define MAX_SCENE_OBJECT_NAME	32
#define SCENE_ROOT_NAME "Scene Root"

//end of Defs.h
