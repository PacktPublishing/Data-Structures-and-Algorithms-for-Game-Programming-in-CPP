cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/KnightGameRegistry.cmake")

if(NOT DEFINED GAME_NAME OR GAME_NAME STREQUAL "")
    message(FATAL_ERROR
        "GAME_NAME is required. Example: "
        "cmake -DGAME_NAME=Game1 -P cmake/AddExistingGame.cmake")
endif()

knight_registry_validate_name("${GAME_NAME}")
knight_registry_resolve_games_dir(games_dir)
knight_registry_validate_game_project("${games_dir}" "${GAME_NAME}" game_dir)
knight_registry_add("${games_dir}" "${GAME_NAME}")

message(STATUS "Registered existing Knight game '${GAME_NAME}'")
message(STATUS "Source directory: ${game_dir}")
message(STATUS "Reconfigure CMake to add the target to generated build files")
