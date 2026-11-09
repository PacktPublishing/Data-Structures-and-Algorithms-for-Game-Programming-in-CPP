include_guard(GLOBAL)

function(knight_registry_validate_name game_name)
    if(NOT game_name MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "Invalid GAME_NAME='${game_name}'. Use a C++ identifier beginning "
            "with a letter or underscore and containing only letters, digits and underscores.")
    endif()
endfunction()

function(knight_registry_resolve_games_dir output_var)
    get_filename_component(repo_root "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/.." ABSOLUTE)
    if(NOT DEFINED KNIGHT_GAMES_DIR OR KNIGHT_GAMES_DIR STREQUAL "")
        set(games_dir "${repo_root}/Games")
    else()
        get_filename_component(games_dir "${KNIGHT_GAMES_DIR}" ABSOLUTE
            BASE_DIR "${repo_root}")
    endif()
    set(${output_var} "${games_dir}" PARENT_SCOPE)
endfunction()

function(knight_registry_read games_dir output_path output_text)
    set(registry "${games_dir}/CMakeLists.txt")
    if(NOT EXISTS "${registry}")
        message(FATAL_ERROR "Knight games registry does not exist: ${registry}")
    endif()

    file(READ "${registry}" registry_text)
    set(begin_marker "# KNIGHT WIZARD GAMES BEGIN")
    set(end_marker "# KNIGHT WIZARD GAMES END")
    string(FIND "${registry_text}" "${begin_marker}" begin_index)
    string(FIND "${registry_text}" "${end_marker}" end_index)
    if(begin_index EQUAL -1 OR end_index EQUAL -1 OR end_index LESS begin_index)
        message(FATAL_ERROR
            "Games registry markers are missing or out of order in ${registry}")
    endif()

    set(${output_path} "${registry}" PARENT_SCOPE)
    set(${output_text} "${registry_text}" PARENT_SCOPE)
endfunction()

function(knight_registry_is_registered registry_text game_name output_var)
    string(REGEX MATCH
        "(^|[\r\n])[ \t]*add_subdirectory\\([ \t]*${game_name}[ \t]*\\)"
        registration_match "${registry_text}")
    if(registration_match)
        set(${output_var} TRUE PARENT_SCOPE)
    else()
        set(${output_var} FALSE PARENT_SCOPE)
    endif()
endfunction()

function(knight_registry_add games_dir game_name)
    knight_registry_read("${games_dir}" registry registry_text)
    knight_registry_is_registered("${registry_text}" "${game_name}" is_registered)
    if(is_registered)
        message(FATAL_ERROR "Knight game '${game_name}' is already registered")
    endif()

    set(end_marker "# KNIGHT WIZARD GAMES END")
    set(registration "add_subdirectory(${game_name})")
    string(REPLACE "${end_marker}"
        "${registration}\n${end_marker}" updated_registry "${registry_text}")
    file(WRITE "${registry}" "${updated_registry}")
endfunction()

function(knight_registry_remove games_dir game_name require_registered)
    knight_registry_read("${games_dir}" registry registry_text)
    knight_registry_is_registered("${registry_text}" "${game_name}" is_registered)
    if(NOT is_registered)
        if(require_registered)
            message(FATAL_ERROR "Knight game '${game_name}' is not registered")
        endif()
        return()
    endif()

    string(REGEX REPLACE
        "(^|[\r\n])[ \t]*add_subdirectory\\([ \t]*${game_name}[ \t]*\\)[ \t]*(\r?\n|$)"
        "\\1" updated_registry "${registry_text}")
    file(WRITE "${registry}" "${updated_registry}")
endfunction()

function(knight_registry_validate_game_project games_dir game_name output_game_dir)
    set(game_dir "${games_dir}/${game_name}")
    get_filename_component(game_dir "${game_dir}" ABSOLUTE)
    get_filename_component(game_parent "${game_dir}" DIRECTORY)
    get_filename_component(games_dir_absolute "${games_dir}" ABSOLUTE)

    if(NOT game_parent STREQUAL games_dir_absolute)
        message(FATAL_ERROR
            "Refusing to operate outside KNIGHT_GAMES_DIR: ${game_dir}")
    endif()
    if(IS_SYMLINK "${game_dir}")
        message(FATAL_ERROR "Refusing to operate on a symbolic link: ${game_dir}")
    endif()
    if(NOT IS_DIRECTORY "${game_dir}")
        message(FATAL_ERROR "Knight game directory does not exist: ${game_dir}")
    endif()

    set(game_cmake "${game_dir}/CMakeLists.txt")
    if(NOT EXISTS "${game_cmake}")
        message(FATAL_ERROR "Knight game CMakeLists.txt does not exist: ${game_cmake}")
    endif()
    file(READ "${game_cmake}" game_cmake_text)
    string(REGEX MATCH
        "NAME[ \t\r\n]+${game_name}([ \t\r\n]|\\))"
        game_name_match "${game_cmake_text}")
    if(NOT game_name_match)
        message(FATAL_ERROR
            "${game_cmake} does not register the expected game name '${game_name}'")
    endif()

    set(${output_game_dir} "${game_dir}" PARENT_SCOPE)
endfunction()
