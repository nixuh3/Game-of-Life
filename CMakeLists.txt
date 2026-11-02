cmake_minimum_required(VERSION 3.28)
project(GameOfLife LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.1.0
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
)
FetchContent_MakeAvailable(SFML)

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
)

add_executable(GameOfLife ${SOURCES})

target_compile_features(GameOfLife PRIVATE cxx_std_23)

target_link_libraries(GameOfLife PRIVATE SFML::Graphics)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT GameOfLife)
