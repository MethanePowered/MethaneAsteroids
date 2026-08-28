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

if (MSVC)
    # FastNoise2 unconditionally installs its "pdb-files/<CONFIG>" directory under MSVC,
    # but sets COMPILE_PDB_NAME for the Debug configuration only, so compile PDBs
    # are written to that directory in Debug builds and it is never created for other configurations.
    # This breaks installation of Release and Profile (RelWithDebInfo) builds with an error:
    #   file INSTALL cannot find ".../_deps/fastnoise2-build/pdb-files/Release": No error.
    # Create these directories in advance to keep "install" target working in all configurations.
    get_cmake_property(IS_MULTI_CONFIG_GENERATOR GENERATOR_IS_MULTI_CONFIG)
    if (IS_MULTI_CONFIG_GENERATOR)
        foreach(CONFIG_NAME ${CMAKE_CONFIGURATION_TYPES})
            file(MAKE_DIRECTORY "${FastNoise2_BINARY_DIR}/pdb-files/${CONFIG_NAME}")
        endforeach()
    else()
        file(MAKE_DIRECTORY "${FastNoise2_BINARY_DIR}/pdb-files")
    endif()
endif()
