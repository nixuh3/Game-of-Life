cmake_minimum_required(VERSION 3.20)

project(game-of-life LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Dependencies
set(RAYLIB_VERSION 6.0)
find_package(raylib ${RAYLIB_VERSION} QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG        6.2
    )

    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED)
        set(FETCHCONTENT_QUIET NO)
        FetchContent_MakeAvailable(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
        set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # or games

    endif()
endif()

set(SOURCES
    src/main.cpp
    src/chunk.cpp
    src/camera.cpp
    src/game.cpp
    src/rules.cpp
    src/simulation.cpp
    src/blueprint.cpp

    src/chunk.hpp
    src/camera.hpp
    src/game.hpp
    src/rules.hpp
    src/simulation.hpp
    src/blueprint.hpp
    src/math.hpp

    # raygui
    src/raygui/raygui.h
    src/raygui/raygui.cpp
)

add_executable(game-of-life ${SOURCES})

target_link_libraries(game-of-life PRIVATE
    raylib
    opengl32
    gdi32
    winmm
)

target_include_directories(game-of-life PRIVATE "src/raygui")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SOURCES})

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT game-of-life)

set(ASSET_SRC "${CMAKE_CURRENT_SOURCE_DIR}/blueprints")
set(ASSET_DST "$<TARGET_FILE_DIR:game-of-life>/blueprints")

add_custom_command(TARGET game-of-life POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSET_SRC} ${ASSET_DST}
)
