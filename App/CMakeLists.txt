set(TARGET MethaneAsteroids)

include(MethaneApplications)
include(MethaneResources)
include(MethaneModules)

add_methane_application(
    TARGET ${TARGET}
    NAME "Methane Asteroids"
    DESCRIPTION "Asteroids sample demonstrates parallel rendering of the asteroids field observable with interactive camera."
    INSTALL_DIR "Apps"
    COPYRIGHT "Copyright 2019-2022 Evgeny Gorodetskiy"
    VERSION "${ASTEROIDS_VERSION_MAJOR}.${ASTEROIDS_VERSION_MINOR}"
    BUILD_NUMBER "${ASTEROIDS_VERSION_BUILD}"
    SOURCES
        AsteroidsApp.h
        AsteroidsApp.cpp
        AsteroidsAppController.h
        AsteroidsAppController.cpp
)

set(TEXTURES_DIR ${RESOURCES_DIR}/Textures)
set(TEXTURES
    ${TEXTURES_DIR}/Planet/Mars.jpg
    ${TEXTURES_DIR}/Galaxy/PositiveX.jpg
    ${TEXTURES_DIR}/Galaxy/NegativeX.jpg
    ${TEXTURES_DIR}/Galaxy/PositiveY.jpg
    ${TEXTURES_DIR}/Galaxy/NegativeY.jpg
    ${TEXTURES_DIR}/Galaxy/PositiveZ.jpg
    ${TEXTURES_DIR}/Galaxy/NegativeZ.jpg
)

add_methane_embedded_textures(${TARGET} "${TEXTURES_DIR}" "${TEXTURES}")

target_link_libraries(${TARGET}
    PRIVATE
        MethaneAsteroidsSimulation
        MethaneBuildOptions
        TaskFlow
)

list(APPEND DEPENDENT_TARGETS MethaneAsteroidsSimulation)
add_prerequisite_binaries(${TARGET} "${DEPENDENT_TARGETS}" "Apps")

set_target_properties(${TARGET}
    PROPERTIES
    FOLDER Apps
)
