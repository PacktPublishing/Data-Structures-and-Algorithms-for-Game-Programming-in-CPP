cmake_minimum_required(VERSION 3.25)

project(Knight
    VERSION 1.0.0
    DESCRIPTION "Knight reusable C++ game framework and cookbook samples"
    LANGUAGES C CXX)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(KNIGHT_BUILD_ALL_GAMES "Build every Knight demo (desktop only by default)" ON)
set(KNIGHT_GAME "Demo1" CACHE STRING "Single game target used by Android and Web builds")

if(ANDROID OR EMSCRIPTEN)
    set(KNIGHT_BUILD_ALL_GAMES OFF CACHE BOOL "Build every Knight demo" FORCE)
endif()

set(BUILD_EXAMPLES OFF CACHE BOOL "Build raylib examples" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "Build raylib games" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
if(ANDROID)
    set(PLATFORM Android CACHE STRING "raylib platform" FORCE)
    set(WITH_PIC ON CACHE BOOL "Build position-independent raylib" FORCE)
elseif(EMSCRIPTEN)
    set(PLATFORM Web CACHE STRING "raylib platform" FORCE)
else()
    set(PLATFORM Desktop CACHE STRING "raylib platform" FORCE)
endif()

add_subdirectory(raylib EXCLUDE_FROM_ALL)
add_subdirectory(Knight)

