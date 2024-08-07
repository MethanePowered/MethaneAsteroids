CPMAddPackage(
    NAME MethaneKit
    GITHUB_REPOSITORY MethanePowered/MethaneKit
    GIT_TAG 4c66c858c5b08f4e20717c4e5f4e09b641a42c73 # Last commit on master branch with latest hot-fixes for v0.7.3
    VERSION ${ASTEROIDS_VERSION_MAJOR}.${ASTEROIDS_VERSION_MINOR}.${ASTEROIDS_VERSION_PATCH}
    OPTIONS
        "METHANE_APPS_BUILD_ENABLED OFF"
        "METHANE_TESTS_BUILD_ENABLED OFF"
        "METHANE_RUN_TESTS_DURING_BUILD OFF"
        "METHANE_VERSION_MAJOR ${ASTEROIDS_VERSION_MAJOR}"
        "METHANE_VERSION_MINOR ${ASTEROIDS_VERSION_MINOR}"
        "METHANE_VERSION_PATCH ${ASTEROIDS_VERSION_PATCH}"
        "METHANE_VERSION_BUILD ${ASTEROIDS_VERSION_BUILD}"
)

list(APPEND CMAKE_MODULE_PATH "${MethaneKit_SOURCE_DIR}/CMake")
