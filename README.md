# Methane Asteroids Sample <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/Logo/MethaneLogoNameSmall.png" width=200 align="right" valign="middle">

[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-ready--to--code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/MethanePowered/MethaneAsteroids)
[![Total lines](https://tokei.rs/b1/github/MethanePowered/MethaneAsteroids)](https://github.com/MethanePowered/MethaneAsteroids)

Asteroids sample demonstrates multi-threaded rendering of large number of random-generated asteroid meshes 
with [Methane Kit](https://github.com/MethanePowered/MethaneKit) library with graphics API and platform independent implementation,
which is based on DirectX 12, Vulkan and Metal.
Thousands of unique asteroid instances (1000-50000) are drawn with individual Draw-calls in parallel with a random combination of:
- random-generated mesh (from array of up to 1000 unique meshes)
- random generated perlin-noise array texture each with 3 projections (from array of up to 50 unique textures)
- random combination of coloring (from 72 color combinations)

Methane Asteroids sample was inspired by [Intel Asteroids D3D12](https://github.com/GameTechDev/asteroids_d3d12),
but implemented from scratch in a cross-platform style using [Methane Kit](https://github.com/MethanePowered/MethaneKit).

[![Open in Gitpod](https://gitpod.io/button/open-in-gitpod.svg)](https://gitpod.io/#https://github.com/MethanePowered/MethaneAsteroids)

| <pre><b>Platform      </b></pre> | <pre><b>Graphics API     </b></pre>    | <pre><b>Screenshot                                </b></pre> |
|-----------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------|
| <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/Platforms/Windows.png" width=24 valign="middle"> Windows | <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/GraphicsApi/DirectX12Small.png" width=24 valign="middle"> DirectX 12<br/><br/><img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/GraphicsApi/VulkanSmall.png" width=24 valign="middle"> Vulkan | ![Asteroids on Windows](Screenshots/AsteroidsWinDirectX12.jpg) |
| <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/Platforms/Ubuntu.png" width=24 valign="middle"> Linux    | <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/GraphicsApi/VulkanSmall.png" width=24 valign="middle"> Vulkan        | ![Asteroids on Linux](Screenshots/AsteroidsLinVulkan.jpg)      |
| <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/Platforms/MacOS.png" width=24 valign="middle"> MacOS     | <img src="https://github.com/MethanePowered/MethaneKit/blob/master/Docs/Images/GraphicsApi/MetalSmall.png" width=24 valign="middle"> Metal          | ![Asteroids on MacOS](Screenshots/AsteroidsMacMetal.jpg)       |

## [Build Instructions](/Build/README.md)

- [Prerequisites](/Build/README.md#prerequisites)
- [Fetch Sources](/Build/README.md#fetch-sources)
  - [Notes](/Build/README.md#notes)
  - [First time initialization](/Build/README.md#first-time-initialization)
  - [Update sources to latest revision](/Build/README.md#update-sources-to-latest-revision)
- [Building from Sources](/Build/README.md#building-from-sources)
  - [Windows Build with Visual Studio](/Build/README.md#windows-build-with-visual-studio)
  - [Linux Build with Unix Makefiles](/Build/README.md#linux-build-with-unix-makefiles)
  - [MacOS Build with XCode](/Build/README.md#macos-build-with-xcode)
- [CMake Generator](/Build/README.md#cmake-generator)
  - [CMake Options](/Build/README.md#cmake-options)
  - [CMake Presets](/Build/README.md#cmake-presets)

## Features

Default parameters of asteroids simulation are selected depending on CPU HW cores count and are displayed 
in right-bottom panel (switched with `F3` key).
Overall scene complexity can be reduced or increased by pressing `[` / `]` keys.
Sample renders galaxy background using [Methane::Graphics::SkyBox](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Extensions/Include/Methane/Graphics/SkyBox.h)
and planet using generated [Methane::Graphics::SphereMesh](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Primitives/Include/Methane/Graphics/Mesh/SphereMesh.hpp)
with spherical texture coordinates. It also uses interactive [Arc-Ball camera](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Camera/Include/Methane/Graphics/ArcBallCamera.h)
rotated with mouse `LMB` and light rotated with `RMB` with keyboard shortcuts also available (see in help by `F1` key).

## Application Controller 

Keyboard actions are enabled with [AsteroidsAppController](/App/AsteroidsAppController.h) 
derived from [Methane::Platform::Keyboard::ActionControllerBase](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Platform/Input/Include/Methane/Platform/KeyboardActionControllerBase.hpp):

| Asteroids App Action         | Keyboard Shortcut   |
|------------------------------|---------------------|
| Switch Parallel Rendering    | `P`                 |
| Switch Mesh LODs Coloring    | `L`                 |
| Increase Mesh LOD Complexity | `'`                 |
| Decrease Mesh LOD Complexity | `;`                 |
| Increase Scene Complexity    | `]`                 |
| Decrease Scene Complexity    | `[`                 |
| Set Scene Complexity 0 .. 9  | `0..9`              |

## Optimizations

Sample includes the following optimizations and features:
- Asteroid meshes use **dynamically selected LODs** depending on estimated screen size.
This allows to greatly reduce GPU overhead. Use `L` key to enable LODs coloring and `'` / `;` keys to increase / reduce overall mesh level of details.
- **Parallel rendering** of asteroids array with individual draw-calls allows to be less CPU bound.
Multi-threading can be switched off for comparing with single-threaded rendering by pressing `P` key.
- **Parallel updating** of asteroid transformation matrices in [AsteroidsArray::Update](/Modules/Simulation/AsteroidsArray.cpp#L352) and 
encoding asteroid meshes rendering in [MeshBuffers::DrawParallel](https://github.com/MethanePowered/MethaneKit/tree/master/Modules/Graphics/Extensions/Include/Methane/Graphics/MeshBuffers.hpp#L160)
are implemented using [Taskflow](https://github.com/taskflow/taskflow/) library which enables effective usage of the thread-pool via `parallel_for` primitive.
- All asteroid textures are bound to program uniform all at once as an **array of textures** to minimize number of program binding calls between draws.
Particular texture is selected on each draw call using index parameter in constants buffer.
Note that each asteroid texture is a texture 2d array itself with 3 mip-mapped textures used for triplane projection.
- **Inverted depth buffer** (with values from 1 in foreground to 0 in background and greater-or-equal compare function)
is used to minimize frame buffer overdrawing by rendering in order from foreground to background: asteroids array with planet
are drawn first and sky-box afterwards.

## Instrumentation and Profiling

Integrated instrumentation of the Methane Kit library and Asteroids sample enables profiling with the following tools:
- [Tracy Profiler](https://github.com/wolfpld/tracy)
- [Intel GPA Trace Analyzer](https://software.intel.com/en-us/gpa/graphics-trace-analyzer)
- [Intel VTune Profiler](https://software.intel.com/content/www/us/en/develop/tools/vtune-profiler.html)

![Asteroids Trace in Tracy](Screenshots/AsteroidsWinTracyProfiling.jpg)
<p align="center"><i>Trace of Asteroids multi-threaded execution on CPU viewed in <a href="https://github.com/wolfpld/tracy">Tracy Profiler</a></i></p>

![Asteroids Trace in GPA Trace Analyzer](Screenshots/AsteroidsWinGPATraceAnalyzer.jpg)
<p align="center"><i>Trace of Asteroids multi-threaded execution on CPU viewed in <a href="https://software.intel.com/en-us/gpa/graphics-trace-analyzer">Intel GPA Trace Analyzer</a></i></p>

## [External Dependencies](/Externals/README.md)

- [Libraries](/Externals/README.md#libraries)
- [Build Tools](/Externals/README.md#build-tools)
- [Assets](/Externals/README.md#assets)

## License

Methane Asteroids sample along with Methane Kit are distributed under [Apache 2.0 License](LICENSE): it is free to use and open for contributions!

*Copyright 2019-2022 © Evgeny Gorodetskiy* [![Follow](https://img.shields.io/twitter/follow/egorodet.svg?style=social)](https://twitter.com/egorodet)
