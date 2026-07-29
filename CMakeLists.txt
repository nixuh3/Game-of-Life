cmake_minimum_required(VERSION 3.20)

project(GameOfLife LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCES
    src/Main.cpp
    src/Chunk.cpp
    src/CameraController.cpp
    src/Game.cpp
    src/Rules.cpp
    src/Simulation.cpp
    
    src/Chunk.h
    src/CameraController.h
    src/Game.h
    src/Rules.h
    src/Simulation.h
    src/Math.h
)

add_executable(GameOfLife ${SOURCES})

target_include_directories(GameOfLife PRIVATE external/raylib-6.0/include)

target_link_directories(GameOfLife PRIVATE external/raylib-6.0/lib)

target_link_libraries(GameOfLife PRIVATE
    raylib
    opengl32
    gdi32
    winmm
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GameOfLife)
