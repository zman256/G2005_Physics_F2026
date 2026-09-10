cmake_minimum_required(VERSION 3.15)
project(physics-1 CXX C)

# Matches the settings used by physics-1.vcxproj / raylib.vcxproj so a CMake
# build behaves the same as the existing Visual Studio solution.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

# ---------------------------------------------------------------------------
# raylib (static library) - compiled directly from source, same file list
# and defines as raylib-5.5/raylib.vcxproj.
# ---------------------------------------------------------------------------
set(RAYLIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/raylib-5.5/src)

add_library(raylib STATIC
    ${RAYLIB_SRC_DIR}/raudio.c
    ${RAYLIB_SRC_DIR}/rcore.c
    ${RAYLIB_SRC_DIR}/rglfw.c
    ${RAYLIB_SRC_DIR}/rmodels.c
    ${RAYLIB_SRC_DIR}/rshapes.c
    ${RAYLIB_SRC_DIR}/rtext.c
    ${RAYLIB_SRC_DIR}/rtextures.c
    ${RAYLIB_SRC_DIR}/utils.c
)

target_include_directories(raylib PUBLIC
    ${RAYLIB_SRC_DIR}
    ${RAYLIB_SRC_DIR}/external/glfw/include
)

target_compile_definitions(raylib PUBLIC
    PLATFORM_DESKTOP
    GRAPHICS_API_OPENGL_43
    _WINSOCK_DEPRECATED_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
)

if (MSVC)
    target_compile_options(raylib PRIVATE /Zc:__cplusplus)
endif()

if (WIN32)
    target_link_libraries(raylib PUBLIC winmm gdi32 opengl32)
endif()

# ---------------------------------------------------------------------------
# physics-1 (game executable) - same source/include/link setup as
# game/physics-1.vcxproj.
# ---------------------------------------------------------------------------
add_executable(physics-1
    game/src/main.cpp
)

target_include_directories(physics-1 PRIVATE
    game
    game/include
)

target_link_libraries(physics-1 PRIVATE raylib)

if (MSVC)
    target_compile_options(physics-1 PRIVATE /Zc:__cplusplus)
endif()

set_target_properties(physics-1 PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/bin/Debug
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/bin/Release
)
