set(TARGET MethanePerlinNoise)

add_library(${TARGET} STATIC
    PerlinNoise.h
    PerlinNoise.cpp
)

target_include_directories(${TARGET}
    PUBLIC
        .
)

target_link_libraries(${TARGET}
    PUBLIC
        MethaneGraphicsTypes
        MethaneInstrumentation
    PRIVATE
        MethaneBuildOptions
        PerlinNoise
)

set_target_properties(${TARGET}
    PROPERTIES
        FOLDER Modules/Asteroids
)
