set(KNIGHT_FRAMEWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Knight")
file(GLOB KNIGHT_FRAMEWORK_SOURCES CONFIGURE_DEPENDS
    "${KNIGHT_FRAMEWORK_DIR}/*.cpp"
    "${KNIGHT_FRAMEWORK_DIR}/*.h")

add_library(knight_framework STATIC ${KNIGHT_FRAMEWORK_SOURCES})
add_library(Knight::Framework ALIAS knight_framework)

if(ANDROID)
    set(KNIGHT_RESOURCE_ROOT "")
else()
    set(KNIGHT_RESOURCE_ROOT "resources/")
endif()

target_include_directories(knight_framework
    PUBLIC
        "$<BUILD_INTERFACE:${KNIGHT_FRAMEWORK_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/raylib/src>"
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/raylib/src/external>")
target_link_libraries(knight_framework PUBLIC raylib)
target_compile_definitions(knight_framework PUBLIC
    KNIGHT_RESOURCE_ROOT_PATH="${KNIGHT_RESOURCE_ROOT}")
set_target_properties(knight_framework PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    FOLDER "Framework")

if(MSVC)
    target_compile_options(knight_framework PRIVATE /W3 /permissive-)
else()
    target_compile_options(knight_framework PRIVATE -Wall -Wextra -Wno-unused-parameter)
    target_compile_options(knight_framework PUBLIC
        -include "${PROJECT_SOURCE_DIR}/cmake/KnightCompatibility.h")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(knight_framework PUBLIC -fms-extensions)
    endif()
endif()

set(KNIGHT_GAME_DIRS
    BonusCoverflow3DUI BonusGameWorld01 BonusGameWorld02 BonusHashGridDemo
    BonusKdTreeDemo BonusMatch3Demo Demo1 Demo10 Demo2a Demo2b Demo2c Demo2d
    Demo2e Demo2f Demo3a Demo3b Demo3c Demo3d Demo3e Demo3f Demo4iso Demo4ss
    Demo4texops Demo53PV Demo5FPC Demo5MultiCams Demo5Ortho Demo5RTSCam
    Demo5TrkCam Demo6Light Demo6LightShader Demo6NormalMap Demo6VertexNormal
    Demo7Billboard Demo7Deferred Demo7HMap Demo7Particles Demo7PCFShadow
    Demo7QuadTreeTerrain Demo7SkyBox Demo8a Demo8b Demo8c Demo9a Demo9b Demo9c)

if(NOT ANDROID AND NOT EMSCRIPTEN)
    add_custom_target(knight_assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/resources"
            "${PROJECT_BINARY_DIR}/bin/$<CONFIG>/resources"
        COMMENT "Copying Knight runtime resources"
        VERBATIM)
    set_target_properties(knight_assets PROPERTIES FOLDER "Framework")
endif()

function(knight_add_game game_name)
    set(game_dir "${CMAKE_CURRENT_SOURCE_DIR}/${game_name}")
    if(NOT IS_DIRECTORY "${game_dir}")
        message(FATAL_ERROR "Knight game directory does not exist: ${game_dir}")
    endif()

    file(GLOB game_sources CONFIGURE_DEPENDS
        "${game_dir}/*.cpp"
        "${game_dir}/*.h")

    if(ANDROID)
        add_library(${game_name} SHARED ${game_sources})
        set_target_properties(${game_name} PROPERTIES OUTPUT_NAME main)
        # NativeActivity looks up this entry point dynamically. Keep the
        # native_app_glue object from being discarded from static raylib.
        target_link_options(${game_name} PRIVATE
            "LINKER:-u,ANativeActivity_onCreate")
    else()
        add_executable(${game_name} ${game_sources})
    endif()

    target_include_directories(${game_name} PRIVATE "${game_dir}")
    target_link_libraries(${game_name} PRIVATE Knight::Framework)
    set_target_properties(${game_name} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${game_name}>"
        XCODE_GENERATE_SCHEME YES
        XCODE_SCHEME_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${game_name}>"
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        FOLDER "Games")

    if(TARGET knight_assets)
        add_dependencies(${game_name} knight_assets)
    endif()

    if(NOT MSVC)
        target_compile_options(${game_name} PRIVATE
            -include "${PROJECT_SOURCE_DIR}/cmake/KnightCompatibility.h")
        if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            target_compile_options(${game_name} PRIVATE -fms-extensions)
        endif()
    endif()

    if(EMSCRIPTEN)
        set_target_properties(${game_name} PROPERTIES SUFFIX ".html")
        target_link_options(${game_name} PRIVATE
            "--shell-file=${PROJECT_SOURCE_DIR}/cmake/web/shell.html"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sASSERTIONS=$<IF:$<CONFIG:Debug>,1,0>")

        if(game_name STREQUAL "Demo1")
            target_link_options(${game_name} PRIVATE
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/shaders@resources/shaders"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/fonts/mecha.png@resources/fonts/mecha.png"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/models/obj/castle.obj@resources/models/obj/castle.obj"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/models/obj/castle_diffuse.png@resources/models/obj/castle_diffuse.png"
                "--preload-file=${PROJECT_SOURCE_DIR}/resources/models/gltf/robot.glb@resources/models/gltf/robot.glb")
        else()
            target_link_options(${game_name} PRIVATE
                "--preload-file=${PROJECT_SOURCE_DIR}/resources@resources")
        endif()
    endif()

    install(TARGETS ${game_name}
        RUNTIME DESTINATION .
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endfunction()

if(KNIGHT_BUILD_ALL_GAMES)
    foreach(game_name IN LISTS KNIGHT_GAME_DIRS)
        knight_add_game(${game_name})
    endforeach()
else()
    if(NOT KNIGHT_GAME IN_LIST KNIGHT_GAME_DIRS)
        message(FATAL_ERROR
            "Unknown KNIGHT_GAME='${KNIGHT_GAME}'. Choose one of: ${KNIGHT_GAME_DIRS}")
    endif()
    knight_add_game(${KNIGHT_GAME})
endif()

if(NOT ANDROID)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/resources"
        DESTINATION .
        PATTERN "*.psd" EXCLUDE)
endif()

