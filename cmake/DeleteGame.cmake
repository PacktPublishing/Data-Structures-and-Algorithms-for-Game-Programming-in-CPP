cmake_minimum_required(VERSION 3.25)
include("${CMAKE_CURRENT_LIST_DIR}/KnightGameRegistry.cmake")

if(NOT DEFINED GAME_NAME OR GAME_NAME STREQUAL "")
    message(FATAL_ERROR
        "GAME_NAME is required. Example: cmake -DGAME_NAME=Game1 "
        "-DCONFIRM_DELETE=Game1 -P cmake/DeleteGame.cmake")
endif()

knight_registry_validate_name("${GAME_NAME}")
if(NOT DEFINED CONFIRM_DELETE OR NOT CONFIRM_DELETE STREQUAL GAME_NAME)
    message(FATAL_ERROR
        "Permanent deletion requires -DCONFIRM_DELETE=${GAME_NAME}. "
        "This action deletes the complete game source directory.")
endif()

knight_registry_resolve_games_dir(games_dir)
knight_registry_validate_game_project("${games_dir}" "${GAME_NAME}" game_dir)

# Unregister first. If deletion fails, the maintained source directory remains
# available and can be restored with AddExistingGame.cmake.
knight_registry_remove("${games_dir}" "${GAME_NAME}" FALSE)
file(REMOVE_RECURSE "${game_dir}")
if(EXISTS "${game_dir}")
    message(FATAL_ERROR
        "Failed to delete ${game_dir}. The game was unregistered, but some files remain. "
        "Inspect the directory and use AddExistingGame.cmake to restore registration if needed.")
endif()

message(STATUS "Permanently deleted Knight game '${GAME_NAME}'")
message(STATUS "Removed directory: ${game_dir}")
message(STATUS "Reconfigure CMake to remove the target from generated build files")
