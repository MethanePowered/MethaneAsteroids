CPMAddPackage(
    NAME FastNoise2
    GITHUB_REPOSITORY MethanePowered/FastNoise2
    VERSION 1.1.1
    OPTIONS
        "FASTNOISE2_NOISETOOL OFF"
        "FASTNOISE2_TESTS OFF"
        "FASTNOISE2_UTILITY OFF"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(FastSIMD_FastNoise
            PRIVATE
            -Wno-ignored-attributes
    )
endif()