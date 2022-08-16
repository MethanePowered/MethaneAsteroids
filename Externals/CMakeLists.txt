list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

# CMake libraries
include(CPM) # keep 1-st
include(CMRC)

# C++ libraries
# Keep MethaneKit first to include MethaneBuildOptions.cmake as early as possible
include(MethaneKit) # keep 1-st
include(PerlinNoise)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} PARENT_SCOPE)