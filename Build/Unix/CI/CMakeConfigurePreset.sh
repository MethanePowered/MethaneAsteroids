#!/bin/bash
# CI script to configure CMake preset
set -o pipefail
mkdir -p "$INSTALL_DIR"
cmake --preset "${1}" \
  -DASTEROIDS_VERSION_MAJOR=${METHANE_VERSION_MAJOR} \
  -DASTEROIDS_VERSION_MINOR=${METHANE_VERSION_MINOR} \
  -DASTEROIDS_VERSION_PATCH=${METHANE_VERSION_PATCH} \
  -DASTEROIDS_VERSION_BUILD=${METHANE_VERSION_BUILD} \
  2>&1 | tee $BUILD_LOG_FILE
