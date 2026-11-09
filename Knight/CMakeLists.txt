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

set(KNIGHT_SAMPLE_DIRS
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

foreach(game_name IN LISTS KNIGHT_SAMPLE_DIRS)
    knight_add_game(
        NAME "${game_name}"
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${game_name}")
endforeach()

if(KNIGHT_BUILD_TESTS AND NOT ANDROID AND NOT EMSCRIPTEN)
    # Runs InputSystem against a scripted backend; no window or GPU needed.
    add_executable(knight_input_tests Tests/InputSystemTests.cpp)
    target_link_libraries(knight_input_tests PRIVATE Knight::Framework)
    set_target_properties(knight_input_tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
        FOLDER "Tests")
    add_test(NAME knight_input_tests COMMAND knight_input_tests)
endif()

if(NOT ANDROID)
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/resources"
        DESTINATION .
        PATTERN "*.psd" EXCLUDE)
endif()

