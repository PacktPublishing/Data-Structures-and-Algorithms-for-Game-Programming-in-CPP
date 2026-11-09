include_guard(GLOBAL)

# Register a Knight game and create its target when requested by the current
# build. All sample and user-created games pass through this function so their
# desktop, Android and Web settings remain consistent.
function(knight_add_game)
    set(options)
    set(one_value_args NAME SOURCE_DIR)
    set(multi_value_args)
    cmake_parse_arguments(GAME
        "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    if(NOT GAME_NAME)
        message(FATAL_ERROR "knight_add_game requires NAME")
    endif()
    if(NOT GAME_SOURCE_DIR)
        message(FATAL_ERROR "knight_add_game(${GAME_NAME}) requires SOURCE_DIR")
    endif()
    if(NOT GAME_NAME MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "Knight game name '${GAME_NAME}' is not a valid CMake/C++ identifier")
    endif()

    get_filename_component(game_dir "${GAME_SOURCE_DIR}" ABSOLUTE)
    if(NOT IS_DIRECTORY "${game_dir}")
        message(FATAL_ERROR "Knight game directory does not exist: ${game_dir}")
    endif()

    get_property(registered_games GLOBAL PROPERTY KNIGHT_REGISTERED_GAMES)
    if(GAME_NAME IN_LIST registered_games)
        message(FATAL_ERROR "Knight game '${GAME_NAME}' is registered more than once")
    endif()
    set_property(GLOBAL APPEND PROPERTY KNIGHT_REGISTERED_GAMES "${GAME_NAME}")
    set_property(GLOBAL PROPERTY "KNIGHT_GAME_SOURCE_DIR_${GAME_NAME}" "${game_dir}")

    if(NOT KNIGHT_BUILD_ALL_GAMES AND NOT GAME_NAME STREQUAL KNIGHT_GAME)
        return()
    endif()

    file(GLOB game_sources CONFIGURE_DEPENDS
        "${game_dir}/*.cpp"
        "${game_dir}/*.h")
    if(NOT game_sources)
        message(FATAL_ERROR "Knight game '${GAME_NAME}' has no .cpp or .h files in ${game_dir}")
    endif()

    if(ANDROID)
        add_library(${GAME_NAME} SHARED ${game_sources})
        set_target_properties(${GAME_NAME} PROPERTIES OUTPUT_NAME main)
        # NativeActivity looks up this entry point dynamically. Keep the
        # native_app_glue object from being discarded from static raylib.
        target_link_options(${GAME_NAME} PRIVATE
            "LINKER:-u,ANativeActivity_onCreate")
    else()
        add_executable(${GAME_NAME} ${game_sources})
    endif()

    target_include_directories(${GAME_NAME} PRIVATE "${game_dir}")
    target_link_libraries(${GAME_NAME} PRIVATE Knight::Framework)
    set_target_properties(${GAME_NAME} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${GAME_NAME}>"
        XCODE_GENERATE_SCHEME YES
        XCODE_SCHEME_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${GAME_NAME}>"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        FOLDER "Games")

    if(TARGET knight_assets)
        add_dependencies(${GAME_NAME} knight_assets)
    endif()

    if(NOT MSVC)
        target_compile_options(${GAME_NAME} PRIVATE
            -include "${PROJECT_SOURCE_DIR}/cmake/KnightCompatibility.h")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${GAME_NAME} PRIVATE -fms-extensions)
        endif()
    endif()

    if(EMSCRIPTEN)
        set_target_properties(${GAME_NAME} PROPERTIES SUFFIX ".html")
        target_link_options(${GAME_NAME} PRIVATE
            "--shell-file=${PROJECT_SOURCE_DIR}/cmake/web/shell.html"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sASSERTIONS=$<IF:$<CONFIG:Debug>,1,0>")

        if(GAME_NAME STREQUAL "Demo1")
            target_link_options(${GAME_NAME} PRIVATE
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/shaders@resources/shaders"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/fonts/mecha.png@resources/fonts/mecha.png"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/models/obj/castle.obj@resources/models/obj/castle.obj"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/models/obj/castle_diffuse.png@resources/models/obj/castle_diffuse.png"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/models/gltf/robot.glb@resources/models/gltf/robot.glb")
        else()
            target_link_options(${GAME_NAME} PRIVATE
                "--preload-file=${PROJECT_SOURCE_DIR}/resources@resources")
        endif()
    endif()

    install(TARGETS ${GAME_NAME}
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endfunction()

function(knight_validate_selected_game)
    get_property(registered_games GLOBAL PROPERTY KNIGHT_REGISTERED_GAMES)
    if(NOT registered_games)
        message(FATAL_ERROR "No Knight games were registered")
    endif()
    list(REMOVE_DUPLICATES registered_games)
    list(SORT registered_games)
    set_property(CACHE KNIGHT_GAME PROPERTY STRINGS ${registered_games})

    if(NOT KNIGHT_BUILD_ALL_GAMES AND NOT KNIGHT_GAME IN_LIST registered_games)
        string(JOIN ", " valid_games ${registered_games})
        message(FATAL_ERROR
            "Unknown KNIGHT_GAME='${KNIGHT_GAME}'. Choose one of: ${valid_games}")
    endif()
endfunction()
