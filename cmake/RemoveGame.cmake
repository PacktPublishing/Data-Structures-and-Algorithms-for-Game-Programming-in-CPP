cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/KnightGameRegistry.cmake")

if(NOT DEFINED GAME_NAME OR GAME_NAME STREQUAL "")
    message(FATAL_ERROR
        "GAME_NAME is required. Example: "
        "cmake -DGAME_NAME=Game1 -P cmake/RemoveGame.cmake")
endif()

knight_registry_validate_name("${GAME_NAME}")
knight_registry_resolve_games_dir(games_dir)
knight_registry_validate_game_project("${games_dir}" "${GAME_NAME}" game_dir)
knight_registry_remove("${games_dir}" "${GAME_NAME}" TRUE)

message(STATUS "Unregistered Knight game '${GAME_NAME}'")
message(STATUS "Source files were preserved: ${game_dir}")
message(STATUS "Reconfigure CMake to remove the target from generated build files")
