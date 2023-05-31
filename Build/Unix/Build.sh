#!/bin/bash
# Run Build.sh with optional arguments:
#   --apple-platform PLATFORM - Apple platform name (MacOS - by default, OS64 - iOS, SIMULATORARM64 - iOS Sim, TVOS, SIMULATOR_TVOS, ...)
#   --apple-dev-team TEAM_ID  - Apple development team id used for code signing (required for iOS platforms)
#   --apple-deploy-target X.Y - minimum version of Apple OS deployment target (15.0 by default)
#   --debug                   - Debug build instead of Release build by default
#   --vulkan VULKAN_SDK       - use Vulkan graphics API via Vulkan SDK path (~/VulkanSDK/1.2.182.0/macOS) instead of Metal on MacOS by default
#   --graphviz                - enable GraphViz cmake module diagrams generation in Dot and Png formats

BUILD_VERSION_MAJOR=0
BUILD_VERSION_MINOR=7
BUILD_VERSION_PATCH=2
BUILD_VERSION=$BUILD_VERSION_MAJOR.$BUILD_VERSION_MINOR.$BUILD_VERSION_PATCH

SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )
SOURCE_DIR=$SCRIPT_DIR/../..
OUTPUT_DIR=$SCRIPT_DIR/../Output
APPLE_DEPLOYMENT_TARGET="16.0"

# Parse command line arguments
while [ $# -ne 0 ]
do
    arg="$1"
    case "$arg" in
        --debug)
            IS_DEBUG_BUILD=true
            ;;
        --vulkan)
            IS_VULKAN_BUILD=true
            VULKAN_SDK="$2"
            shift
            ;;
        --graphviz)
            IS_GRAPHVIZ_BUILD=true
            ;;
        --apple-platform)
            APPLE_PLATFORM="$2"
            shift
            ;;
        --apple-dev-team)
            APPLE_DEVELOPMENT_TEAM="$2"
            shift
            ;;
        --apple-deploy-target)
            APPLE_DEPLOYMENT_TARGET="$2"
            shift
            ;;
        *)
            echo "Unknown argument: $arg" && exit 1
            ;;
    esac
    shift
done

# Choose CMake generator depending on operating system
ARCH_NAME="$(uname -m)"
OS_NAME="$(uname -s)"
case "${OS_NAME}" in
    Linux*)
        CMAKE_GENERATOR=Unix\ Makefiles
        PLATFORM_NAME=Linux
        ;;
    Darwin*)
        CMAKE_GENERATOR=Xcode
        PLATFORM_NAME=MacOS
        if [ "$APPLE_PLATFORM" != "" ]; then
            # Disable tests cause unbundled console executables can not be built with iOS toolchain
            TESTS_BUILD_ENABLED="OFF"
            CMAKE_FLAGS="-DCMAKE_TOOLCHAIN_FILE=$SOURCE_DIR/Externals/iOS-Toolchain.cmake \
                         -DPLATFORM=$APPLE_PLATFORM \
                         -DDEPLOYMENT_TARGET=$APPLE_DEPLOYMENT_TARGET \
                         -DENABLE_ARC:BOOL=ON \
                         -DENABLE_VISIBILITY:BOOL=ON \
                         -DENABLE_BITCODE:BOOL=OFF \
                         -DENABLE_STRICT_TRY_COMPILE:BOOL=OFF"
            if [ "$APPLE_DEVELOPMENT_TEAM" != "" ]; then
                CMAKE_FLAGS="$CMAKE_FLAGS \
                         -DAPPLE_DEVELOPMENT_TEAM=${APPLE_DEVELOPMENT_TEAM}"
                CMAKE_BUID_OPTIONS="-- -allowProvisioningUpdates"
            fi
        else
            APPLE_PLATFORM=MacOS_$ARCH_NAME
        fi
        ;;
    *)
    echo "Unsupported operating system!" 1>&2 && exit 1
    ;;
esac

if [ "$IS_DEBUG_BUILD" == true ]; then
    BUILD_TYPE=Debug
else
    BUILD_TYPE=Release
fi

if [ "$IS_VULKAN_BUILD" == true ]; then
    VULKAN_BUILD_FLAG=ON
    GFX_API_NAME=Vulkan
    GFX_API=VK
    if [ ! -d "$VULKAN_SDK" ]; then
        echo "Vulkan SDK path does not exist: $VULKAN_SDK" && exit 1
    fi
else
    VULKAN_BUILD_FLAG=OFF
    GFX_API_NAME=Metal
    GFX_API=MT
fi

CONFIG_DIR=$OUTPUT_DIR/$CMAKE_GENERATOR
if [ "$APPLE_PLATFORM" != "" ]; then
    CONFIG_DIR=$CONFIG_DIR/$APPLE_PLATFORM
fi

CONFIG_DIR=$CONFIG_DIR/$GFX_API-$BUILD_TYPE
INSTALL_DIR=$CONFIG_DIR/Install

if [[ "$CMAKE_GENERATOR" == "Xcode" && "$APPLE_PLATFORM" =~ ^MacOS.*$ ]]; then
    # Build architectures have to be set explicitly via generator command line starting with XCode and Clang v12
    CLANG_VERSION="$(clang --version | grep -o -E 'version [0-9]+\.' | grep -o -E '[0-9]+')"
    if [ $CLANG_VERSION -ge 12 ]; then
        CMAKE_FLAGS="$CMAKE_FLAGS -DCMAKE_OSX_ARCHITECTURES=$ARCH_NAME"
    fi
fi

CMAKE_FLAGS="$CMAKE_FLAGS \
    -DMETHANE_VERSION_MAJOR=$BUILD_VERSION_MAJOR \
    -DMETHANE_VERSION_MINOR=$BUILD_VERSION_MINOR \
    -DMETHANE_VERSION_PATCH=$BUILD_VERSION_PATCH \
    -DMETHANE_GFX_VULKAN_ENABLED:BOOL=$VULKAN_BUILD_FLAG \
    -DMETHANE_SHADERS_CODEVIEW_ENABLED:BOOL=ON \
    -DMETHANE_RHI_PIMPL_INLINE_ENABLED:BOOL=ON \
    -DMETHANE_PRECOMPILED_HEADERS_ENABLED:BOOL=ON \
    -DMETHANE_RUN_TESTS_DURING_BUILD:BOOL=OFF \
    -DMETHANE_COMMAND_DEBUG_GROUPS_ENABLED:BOOL=ON \
    -DMETHANE_LOGGING_ENABLED:BOOL=OFF \
    -DMETHANE_OPEN_IMAGE_IO_ENABLED:BOOL=OFF \
    -DMETHANE_SCOPE_TIMERS_ENABLED:BOOL=OFF \
    -DMETHANE_ITT_INSTRUMENTATION_ENABLED:BOOL=ON \
    -DMETHANE_ITT_METADATA_ENABLED:BOOL=OFF \
    -DMETHANE_GPU_INSTRUMENTATION_ENABLED:BOOL=OFF \
    -DMETHANE_TRACY_PROFILING_ENABLED:BOOL=OFF \
    -DMETHANE_TRACY_PROFILING_ON_DEMAND:BOOL=OFF"

if [ "$IS_GRAPHVIZ_BUILD" == true ]; then
    GRAPHVIZ_DIR=$CONFIG_DIR/GraphViz
    GRAPHVIZ_DOT_DIR=$GRAPHVIZ_DIR/dot
    GRAPHVIZ_IMG_DIR=$GRAPHVIZ_DIR/img
    GRAPHVIZ_FILE=MethaneAsteroids.dot
    GRAPHVIZ_DOT_EXE=$(which dot)
    CMAKE_FLAGS="$CMAKE_FLAGS --graphviz=$GRAPHVIZ_DOT_DIR/$GRAPHVIZ_FILE"
fi

BUILD_DIR=$CONFIG_DIR/Build
echo =========================================================
echo Clean build and install Methane Asteroids $GFX_API_NAME $BUILD_TYPE
echo =========================================================
echo  \* Build in:   $BUILD_DIR
echo  \* Install to: $INSTALL_DIR
if [ "$IS_GRAPHVIZ_BUILD" == true ]; then
    echo  \* Graphviz in: $GRAPHVIZ_DIR
fi
echo =========================================================

rm -rf "$CONFIG_DIR"
if ! mkdir -p "$BUILD_DIR"; then
    echo "Failed to create clean build directory."
    exit 1
fi

echo ----------
echo Generating build files for $CMAKE_GENERATOR...
if ! cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -G "$CMAKE_GENERATOR" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" $CMAKE_FLAGS; then
    echo "Methane CMake generation failed."
    exit 1
fi

if [ "$IS_GRAPHVIZ_BUILD" == true ]; then
    echo ----------
    if [ -x "$GRAPHVIZ_DOT_EXE" ]; then
        echo Converting GraphViz diagrams to images...
        mkdir "$GRAPHVIZ_IMG_DIR"
        for DOT_PATH in $GRAPHVIZ_DOT_DIR/*; do
            DOT_IMG=$GRAPHVIZ_IMG_DIR/${DOT_PATH##*/}.png
            echo Writing image $DOT_IMG...
            if ! "$GRAPHVIZ_DOT_EXE" -Tpng "$DOT_PATH" -o "$DOT_IMG"; then
                echo "Dot failed to generate diagram image."
                exit 1
            fi
        done
    else
        echo GraphViz dot executable was not found. Skipping graph images generation.
    fi
fi

echo ----------
echo Build with $CMAKE_GENERATOR...
if ! cmake --build "$BUILD_DIR" --config $BUILD_TYPE --target install --parallel 8 $CMAKE_BUID_OPTIONS; then
    echo "Methane Asteroids build failed."
    exit 1
fi
