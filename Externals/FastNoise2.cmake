CPMAddPackage(
    NAME FastNoise2
    GITHUB_REPOSITORY MethanePowered/FastNoise2
    GIT_TAG f8facbad699a51f0b5a0800223d0813cca1d34be
    VERSION 0.10.0-alpha
    OPTIONS
        "FASTNOISE2_NOISETOOL OFF"
        "FASTNOISE2_TESTS OFF"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(FastNoise PRIVATE -Wno-overflow)
endif()