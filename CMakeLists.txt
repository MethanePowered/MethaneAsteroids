cmake_minimum_required(VERSION 3.21.0)

# Methane version, build & product info
set(ASTEROIDS_VERSION_MAJOR 0 CACHE STRING "Asteroids major version")
set(ASTEROIDS_VERSION_MINOR 8 CACHE STRING "Asteroids minor version")
set(ASTEROIDS_VERSION_PATCH 1 CACHE STRING "Asteroids patch version")
set(ASTEROIDS_VERSION_BUILD 0 CACHE STRING "Asteroids build version")

project(METHANE_ASTEROIDS
        VERSION ${ASTEROIDS_VERSION_MAJOR}.${ASTEROIDS_VERSION_MINOR}.${ASTEROIDS_VERSION_PATCH}.${ASTEROIDS_VERSION_BUILD}
        DESCRIPTION "Asteroids sample demonstrating multi-threaded rendering of large number of heterogeneous asteroids with Methane Kit using DirectX 12, Vulkan and Metal"
        HOMEPAGE_URL "https://github.com/MethanePowered/MethaneAsteroids"
        LANGUAGES CXX C
)

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW) # option honors normal variables
endif()

set(CMAKE_CXX_STANDARD 20)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Use build-independent location for CPM package source cache to speedup CMake configuration
# NOTE: Do not use single cache location under CLion IDE to workaround CPM cache initialization collision
#       when multiple CMake configurations are initialized simultaneously
if (NOT CPM_SOURCE_CACHE AND NOT DEFINED ENV{CLION_IDE})
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/Build/Output/ExternalsCache" CACHE STRING "Set default CPM source cache path for External repositories")
endif()

if(WIN32)
    set(VS_STARTUP_PROJECT MethaneAsteroids)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") # use /MT static runtime linking
endif()

set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")

add_subdirectory(Externals)

include(MethaneGlobalOptions)
include(MethaneModules)

get_native_graphics_apis()
get_target_property(METHANE_GFX_API MethaneKit GRAPHICS_API)
get_target_property(DXC_BINARY_DIR MethaneKit DXC_BINARY_DIR)
get_target_property(SPIRV_BINARY_DIR MethaneKit SPIRV_BINARY_DIR)

add_subdirectory(Modules)
add_subdirectory(App)
