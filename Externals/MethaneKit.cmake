CPMAddPackage(
    NAME MethaneKit
    GITHUB_REPOSITORY MethanePowered/MethaneKit
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
