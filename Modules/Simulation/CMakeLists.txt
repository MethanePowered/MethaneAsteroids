include(MethaneShaders)

set(TARGET MethaneAsteroidsSimulation)

add_library(${TARGET} STATIC
    Asteroid.h
    Asteroid.cpp
    AsteroidsArray.h
    AsteroidsArray.cpp
    Planet.h
    Planet.cpp
    Shaders/SceneConstants.h
    Shaders/AsteroidUniforms.h
    Shaders/PlanetUniforms.h
)

target_include_directories(${TARGET} PUBLIC .)

target_link_libraries(${TARGET}
    PUBLIC
        MethaneKit
    PRIVATE
        MethanePerlinNoise
        MethaneBuildOptions
        TaskFlow
)

set_target_properties(${TARGET}
    PROPERTIES
    FOLDER Apps
)

include(MethaneShaders)

add_methane_shaders_source(
    TARGET ${TARGET}
    SOURCE Shaders/Asteroids.hlsl
    VERSION 6_0
    TYPES
        vert=AsteroidVS:TEXTURES_COUNT=1
        frag=AsteroidPS:TEXTURES_COUNT=1
        vert=AsteroidVS:TEXTURES_COUNT=5
        frag=AsteroidPS:TEXTURES_COUNT=5
        vert=AsteroidVS:TEXTURES_COUNT=10
        frag=AsteroidPS:TEXTURES_COUNT=10
        vert=AsteroidVS:TEXTURES_COUNT=20
        frag=AsteroidPS:TEXTURES_COUNT=20
        vert=AsteroidVS:TEXTURES_COUNT=30
        frag=AsteroidPS:TEXTURES_COUNT=30
        vert=AsteroidVS:TEXTURES_COUNT=40
        frag=AsteroidPS:TEXTURES_COUNT=40
        vert=AsteroidVS:TEXTURES_COUNT=50
        frag=AsteroidPS:TEXTURES_COUNT=50
)

add_methane_shaders_source(
    TARGET ${TARGET}
    SOURCE Shaders/Planet.hlsl
    VERSION 6_0
    TYPES
        frag=PlanetPS
        vert=PlanetVS
)

add_methane_shaders_library(${TARGET})
