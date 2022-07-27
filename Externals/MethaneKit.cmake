CPMAddPackage(
    NAME MethaneKit
    GITHUB_REPOSITORY MethanePowered/MethaneKit
    GIT_TAG 6550a1a4dce5b86a2dee701c7a96b03cbdc3a9ed # last commit in develop branch
    VERSION 0.6.20220722.6
    OPTIONS
        "METHANE_APPS_BUILD_ENABLED OFF"
        "METHANE_TESTS_BUILD_ENABLED OFF"
        "METHANE_RUN_TESTS_DURING_BUILD OFF"
)

list(APPEND CMAKE_MODULE_PATH "${MethaneKit_SOURCE_DIR}/CMake")
