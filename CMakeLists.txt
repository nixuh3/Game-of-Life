cmake_minimum_required(VERSION 3.20)

project(game-of-life LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.2
)

FetchContent_MakeAvailable(raylib)

set(SOURCES
    src/main.cpp
    src/chunk.cpp
    src/camera.cpp
    src/game.cpp
    src/rules.cpp
    src/simulation.cpp

    src/chunk.hpp
    src/camera.hpp
    src/game.hpp
    src/rules.hpp
    src/simulation.hpp
    src/math.hpp
)

add_executable(game-of-life ${SOURCES})

target_link_libraries(game-of-life PRIVATE
    raylib
    opengl32
    gdi32
    winmm
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT game-of-life)
