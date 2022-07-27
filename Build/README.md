# Methane Asteroids Build Instructions 

- [Prerequisites](#prerequisites)
- [Fetch Sources](#fetch-sources)
  - [First time initialization](#first-time-initialization)
  - [Update sources to latest revision](#update-sources-to-latest-revision)
- [Building from Sources](#building-from-sources)
  - [Windows Build with Visual Studio](#windows-build-with-visual-studio)
  - [Linux Build with Unix Makefiles](#linux-build-with-unix-makefiles)
  - [MacOS Build with XCode](#macos-build-with-xcode)
- [CMake Generator](#cmake-generator)
  - [CMake Options](#cmake-options)
  - [CMake Presets](#cMake-presets)

## Prerequisites

- **Common**
  - [CMake](https://cmake.org/download/) 3.18 or later
  - [Git](https://git-scm.com/downloads) (is used to fetch external dependent repositories using CMake)
- **Windows**
  - Windows 10 RS5 (build 1809) or later
  - Visual Studio 2019/22 with MSVC v142 or later
  - Windows 10 SDK (latest)
- **MacOS**
  - MacOS 10.15 "Catalina" or later
  - XCode 11 or later with command-line tools
- **Linux**
  - Ubuntu 20.04 or later
  - GCC 9 or later
  - X11, XCB, LCov (optional for code coverage) libraries
  ```console
  sudo apt-get update && sudo apt-get install cmake g++ lcov xcb libx11-dev libx11-xcb-dev libxcb-randr0-dev
  ```

## Fetch Sources

### Notes
- Since v0.6 Methane Kit does not use Git submodules anymore and switches to [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)
to fetch dependent repositories during CMake configuration stage, so it should be both possible to acquire sources
with `git clone` command or to download as ZIP-archive using `Code > Download ZIP` button.
- All [External](/Externals) dependencies are fetched to the `Build/Output/ExternalsCache/...` directory,
which can be changed by adding `-DCPM_SOURCE_CACHE=<cache_path>` to the CMake configuration command.
- Consider using <ins>short path for repository location on Windows</ins> (for example `c:\Git\`),
which may be required to resolve problem with support of paths longer than 260 symbols for some of Microsoft build tools.

### First time initialization

```console
git clone https://github.com/MethanePowered/MethaneAsteroids.git
cd MethaneAsteroids
```

### Update sources to latest revision

```console
cd MethaneAsteroids
git pull
```

## Building from Sources

### Windows Build with Visual Studio

Start Command Prompt, go to `MethaneAsteroids` root directory and either start auxiliary build script
[Build/Windows/Build.bat](/Build/Windows/Build.bat) or build with CMake command line
to generate Visual Studio 2019/22 solution:

```console
set OUTPUT_DIR=Build\Output\VisualStudio\Win64-DX
cmake -S . -B %OUTPUT_DIR%\Build -G "Visual Studio 17 2022" -A x64 -DCMAKE_INSTALL_PREFIX="%cd%\%OUTPUT_DIR%\Install"
cmake --build %OUTPUT_DIR%\Build --config Release --target install
```

Alternatively root [CMakeLists.txt](/CMakeLists.txt) can be opened directly in Visual Studio or 
any other IDE with native CMake support and [built using CMake presets](#cmake-presets).

[Methane Graphics Core](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Core) is built using **DirectX 12** graphics API by default on Windows. 
Vulkan graphics API can be used instead by adding cmake generator option `-DMETHANE_GFX_VULKAN_ENABLED:BOOL=ON` or 
by running `Build/Windows/Build.bat --vulkan`.

Run built applications from the installation directory `Build\Output\VisualStudio\Win64-DX\Install\Apps`

###  Linux Build with Unix Makefiles

Start Terminal, go to `MethaneAsteroids` root directory and either start auxiliary build script
[Build/Unix/Build.sh](/Build/Unix/Build.sh) or build with CMake command line to generate Unix Makefiles:

```console
OUTPUT_DIR=Build/Output/Linux
cmake -S . -B $OUTPUT_DIR/Build -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX="$(pwd)/$OUTPUT_DIR/Install"
cmake --build $OUTPUT_DIR/Build --config Release --target install --parallel 8
```

[Methane Graphics Core](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Core) is built using **Vulkan** graphics API on Linux.

Alternatively root [CMakeLists.txt](/CMakeLists.txt) can be opened directly in
any IDE with native CMake support and [built using CMake presets](#cmake-presets).

Run built applications from the installation directory `Build/Output/Linux/Install/Apps`.
Note that in Ubuntu Linux even GUI applications should be started from "Terminal" app,
because of `noexec` permission set on user's home directory by security reasons.

### MacOS Build with XCode

Start Terminal, go to `MethaneAsteroids` root directory and either start auxiliary build script
[Build/Unix/Build.sh](/Build/Unix/Build.sh) or build with CMake command line to generate XCode workspace:

```console
OUTPUT_DIR=Build/Output/XCode
cmake -S . -B $OUTPUT_DIR/Build -G Xcode -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DCMAKE_INSTALL_PREFIX="$(pwd)/$OUTPUT_DIR/Install"
cmake --build $OUTPUT_DIR/Build --config Release --target install
```

Note that starting with XCode 12 and Clang 12 build architectures have to be specified explicitly
using CMake generator command line option `-DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"` to build the fat binary.
This option should be omitted with earlier versions of Clang on macOS.

Alternatively root [CMakeLists.txt](/CMakeLists.txt) can be opened directly in Visual Studio or 
any other IDE with native CMake support and [built using CMake presets](#cmake-presets).

[Methane Graphics Core](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Core) is built using **Metal** graphics API on MacOS by default.
Vulkan graphics API can be used instead by adding cmake generator option `-DMETHANE_GFX_VULKAN_ENABLED:BOOL=ON` or
by running `Build/Unix/Build.sh --vulkan`, but it requires Vulkan SDK installation with MoltenVK driver implementation
on top of Metal, which is not currently supporting all extensions required by Methane Kit.

Run built applications from the installation directory `Build/Output/XCode/Install/Apps`.

## CMake Generator

### CMake Options

[Methane Kit CMake options](https://github.com/MethanePowered/MethaneKit/tree/master/Build#cmake-options) can be used in cmake generator command line:

```console
cmake -G [Generator] ... -D[BUILD_OPTION_NAME]:BOOL=[ON|OFF]
```

### CMake Presets

[CMake Presets](/CMakePresets.json) can be used to configure and build project with a set of predefined options (CMake 3.20 is required):
```console
cmake --preset [ConfigPresetName]
cmake --build --preset [BuildPresetName] --target install
```

Configure preset names `[ConfigPresetName]` can be listed with `cmake --list-presets` and are constructed according to the next schema using compatible kets according to preset matrix:
```console
[ConfigPresetName] = [VS2019|Xcode|Make|Ninja]-[Win64|Win32|Win|Lin|Mac]-[DX|VK|MTL]-[Default|Profile|Scan]
```

| Preset Matrix | VS2019    | Xcode     | Make      | Ninja     |   
|---------------|-----------|-----------|-----------|-----------|
| Win64         | DX / VK   | -         | -         | -         |
| Win32         | DX / VK   | -         | -         | -         |
| Win           | -         | -         | -         | DX / VK   |
| Mac           | -         | MTL       | -         | MTL       |
| Lin           | -         | -         | VK        | VK        |

Build preset names `[BuildPresetName]` can be listed with `cmake --list-presets build` and are constructed according to the same schema, 
but `Default` suffix should be replaced with `Debug` or `Release` configuration name. Only compatible configure and build presets 
can be used together either with the same name, or with `Debug` or `Release` instead of `Default`. `Ninja` presets should be used from 
"x64/x86 Native Tools Command Prompt for VS2019" command line environment on Windows or directly from Visual Studio.
