if (APPLE)
    set(FASTSIMD_COMPILE_AARCH64 true)
    set(FASTSIMD_COMPILE_ARM true)
    set(FASTSIMD_COMPILE_HAVE_NEON true)
endif()

CPMAddPackage(
    NAME FastNoise2
    GITHUB_REPOSITORY MethanePowered/FastNoise2
    VERSION 0.10.0-alpha
    OPTIONS
        "FASTNOISE2_NOISETOOL OFF"
        "FASTNOISE2_TESTS OFF"
)