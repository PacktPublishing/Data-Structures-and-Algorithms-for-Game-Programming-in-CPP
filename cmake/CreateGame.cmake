cmake_minimum_required(VERSION 3.25)

set(_supported_templates empty)

if(LIST_GAME_TEMPLATES)
    message(STATUS "Available Knight game templates:")
    message(STATUS "  empty - Minimal Knight application with centered Hello text")
    return()
endif()

if(NOT DEFINED GAME_NAME OR GAME_NAME STREQUAL "")
    message(FATAL_ERROR
        "GAME_NAME is required. Example: "
        "cmake -DGAME_NAME=Game1 -P cmake/CreateGame.cmake")
endif()
if(NOT GAME_NAME MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
    message(FATAL_ERROR
        "Invalid GAME_NAME='${GAME_NAME}'. Use a C++ identifier beginning "
        "with a letter or underscore and containing only letters, digits and underscores.")
endif()

if(NOT DEFINED GAME_TEMPLATE OR GAME_TEMPLATE STREQUAL "")
    set(GAME_TEMPLATE empty)
endif()
if(NOT GAME_TEMPLATE IN_LIST _supported_templates)
    string(JOIN ", " _template_names ${_supported_templates})
    message(FATAL_ERROR
        "Unknown GAME_TEMPLATE='${GAME_TEMPLATE}'. Choose one of: ${_template_names}")
endif()

get_filename_component(_repo_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
if(NOT DEFINED KNIGHT_GAMES_DIR OR KNIGHT_GAMES_DIR STREQUAL "")
    set(KNIGHT_GAMES_DIR "${_repo_root}/Games")
endif()
get_filename_component(KNIGHT_GAMES_DIR "${KNIGHT_GAMES_DIR}" ABSOLUTE
    BASE_DIR "${_repo_root}")

set(_registry "${KNIGHT_GAMES_DIR}/CMakeLists.txt")
set(_template_dir "${CMAKE_CURRENT_LIST_DIR}/templates/${GAME_TEMPLATE}")
set(_game_dir "${KNIGHT_GAMES_DIR}/${GAME_NAME}")
set(_stage_dir "${KNIGHT_GAMES_DIR}/.knight-create-${GAME_NAME}")

if(NOT EXISTS "${_registry}")
    message(FATAL_ERROR "Knight games registry does not exist: ${_registry}")
endif()
if(NOT IS_DIRECTORY "${_template_dir}")
    message(FATAL_ERROR "Knight game template does not exist: ${_template_dir}")
endif()
if(EXISTS "${_game_dir}")
    message(FATAL_ERROR "Knight game already exists: ${_game_dir}")
endif()

# A stale staging folder can only have this exact wizard-owned name under the
# configured Games directory. Remove it before beginning a fresh transaction.
if(EXISTS "${_stage_dir}")
    file(REMOVE_RECURSE "${_stage_dir}")
endif()
file(MAKE_DIRECTORY "${_stage_dir}")

configure_file("${_template_dir}/CMakeLists.txt.in"
    "${_stage_dir}/CMakeLists.txt" @ONLY NEWLINE_STYLE LF)
configure_file("${_template_dir}/Game.h.in"
    "${_stage_dir}/${GAME_NAME}.h" @ONLY NEWLINE_STYLE LF)
configure_file("${_template_dir}/Game.cpp.in"
    "${_stage_dir}/${GAME_NAME}.cpp" @ONLY NEWLINE_STYLE LF)

foreach(_required_file IN ITEMS
        CMakeLists.txt "${GAME_NAME}.h" "${GAME_NAME}.cpp")
    if(NOT EXISTS "${_stage_dir}/${_required_file}")
        file(REMOVE_RECURSE "${_stage_dir}")
        message(FATAL_ERROR "Failed to generate ${_required_file}")
    endif()
endforeach()

file(READ "${_registry}" _registry_text)
set(_begin_marker "# KNIGHT WIZARD GAMES BEGIN")
set(_end_marker "# KNIGHT WIZARD GAMES END")
string(FIND "${_registry_text}" "${_begin_marker}" _begin_index)
string(FIND "${_registry_text}" "${_end_marker}" _end_index)
if(_begin_index EQUAL -1 OR _end_index EQUAL -1 OR _end_index LESS _begin_index)
    file(REMOVE_RECURSE "${_stage_dir}")
    message(FATAL_ERROR
        "Games registry markers are missing or out of order in ${_registry}")
endif()

set(_registration "add_subdirectory(${GAME_NAME})")
string(FIND "${_registry_text}" "${_registration}" _existing_registration)
if(NOT _existing_registration EQUAL -1)
    file(REMOVE_RECURSE "${_stage_dir}")
    message(FATAL_ERROR "Knight game '${GAME_NAME}' is already registered")
endif()

file(RENAME "${_stage_dir}" "${_game_dir}" RESULT _rename_result)
if(NOT _rename_result STREQUAL "0")
    file(REMOVE_RECURSE "${_stage_dir}")
    message(FATAL_ERROR "Could not create ${_game_dir}: ${_rename_result}")
endif()

string(REPLACE "${_end_marker}"
    "${_registration}\n${_end_marker}" _updated_registry "${_registry_text}")
file(WRITE "${_registry}" "${_updated_registry}")

message(STATUS "Created Knight game '${GAME_NAME}' from template '${GAME_TEMPLATE}'")
message(STATUS "Location: ${_game_dir}")
message(STATUS "Windows: cmake --preset vs2022")
message(STATUS "         cmake --build --preset vs2022-debug --target ${GAME_NAME}")
message(STATUS "macOS:   cmake --preset xcode")
message(STATUS "         cmake --build --preset xcode-debug --target ${GAME_NAME}")
message(STATUS "Web:     cmake --preset web-debug -DKNIGHT_GAME=${GAME_NAME}")
message(STATUS "         cmake --build --preset web-debug --target ${GAME_NAME}")
message(STATUS "Android: gradlew :app:assembleDebug -PknightGame=${GAME_NAME}")
