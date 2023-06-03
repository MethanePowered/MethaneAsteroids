# Methane Asteroids External Dependencies

All external [libraries](#libraries) and [build tools](#build-tools)
are integrated using [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) package manager script.
Specific versions of the dependent repositories are cloned and configured
during `MethaneKit` CMake configuration step either to the `Build/Output/ExternalsCache` directory (by default)
or to the custom location provided via CMake option `CPM_SOURCE_CACHE`.
Under CLion IDE dependent repositories are cloned inside cmake build directory of each configuration 
to support parallel cmake configurations processing.

## Libraries

| Libraries                                                  | Version | Linkage     | License                                                                                   | Description                                                                                                                                    |
|------------------------------------------------------------|---------|-------------|-------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------------------------------------|
| [MethaneKit](https://github.com/MethanePowered/MethaneKit) | 0.7.2   | Static      | [Apache 2.0](https://github.com/MethanePowered/MethaneKit/blob/master/LICENSE)            | Modern 3D graphics made simple with C++17 cross-platform framework and rendering abstraction API on top of DirectX 12, Metal & Vulkan.         |
| [FastNoise2](https://github.com/Auburn/FastNoise2)         | 0.10.0  | Static      | [MIT](https://github.com/Auburn/FastNoise2/blob/master/LICENSE)                           | Modular node graph based noise generation library using SIMD, C++17 and templates.                                                             |
| [HLSL++](https://github.com/redorav/hlslpp)                | 3.3.1   | Header-only | [MIT](https://github.com/MethanePowered/HLSLpp/blob/master/LICENSE)                       | Math library using hlsl syntax with SSE/NEON support.                                                                                          |
| [Tracy](https://github.com/wolfpld/tracy)                  | 0.9.1   | Static      | [BSD 3.0](https://github.com/wolfpld/tracy/blob/master/LICENSE)                           | A real time, nanosecond resolution, remote telemetry, hybrid frame and sampling profiler for games and other applications.                     |
| [ITT API](https://github.com/intel/ittapi)                 | 3.23.0  | Static      | [BSD 3.0](https://github.com/MethanePowered/IttApi/blob/master/LICENSES/BSD-3-Clause.txt) | Intel® Instrumentation and Tracing Technology (ITT) and Just-In-Time (JIT) API.                                                                |
| [Magic Enum](https://github.com/Neargye/magic_enum)        | 0.8.0   | Header-only | [MIT](https://github.com/Neargye/magic_enum/blob/master/LICENSE)                          | Static reflection for enums (to string, from string, iteration) for modern C++, work with any enum type without any macro or boilerplate code. |
| [TaskFlow](https://github.com/taskflow/taskflow)           | 3.4.0   | Header-only | [MIT](https://github.com/taskflow/taskflow/blob/master/LICENSE)                           | A General-purpose Parallel and Heterogeneous Task Programming System.                                                                          |

## Build Tools

| Libraries                                                                     | Version   | Usage                    | License                                                                                  | Description                                                                      |
|-------------------------------------------------------------------------------|-----------|--------------------------|------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------|
| [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake)                           | 0.38.1    | CMake                    | [MIT](https://github.com/cpm-cmake/CPM.cmake/blob/master/LICENSE)                        | CMake's missing package manager.                                                 |
| [CMRC](https://github.com/vector-of-bool/cmrc)                                | n/a       | CMake                    | [MIT](https://github.com/vector-of-bool/cmrc/blob/master/LICENSE.txt)                    | A Resource Compiler in a Single CMake script.                                    |
| [iOS-Toolchain.cmake](https://github.com/leetal/ios-cmake)                    | 4.3.0     | CMake                    | [BSD 3.0](https://github.com/leetal/ios-cmake/blob/master/LICENSE.md)                    | A CMake toolchain file for iOS, macOS, watchOS & tvOS C/C++/Obj-C++ development. |
| [DirectX Shader Compiler](https://github.com/microsoft/DirectXShaderCompiler) | 1.6.2104  | HLSL Compiler to SPIRV   | [LLVM Release](https://github.com/microsoft/DirectXShaderCompiler/blob/main/LICENSE.TXT) | Open-source HLSL Shader Compiler which is based on LLVM/Clang.                   |
| [SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)                    | 1.3.216.0 | SPIRV Converter to Metal | [Apache 2.0](https://github.com/KhronosGroup/SPIRV-Cross/blob/master/LICENSE)            | Practical tool for disassembling SPIR-V back to high level languages.            |

## Assets

| Type     | Asset Name                                                         | Copyright                                              | License                                                   | Description                                                                                                                                                                                                                                                             |
|----------|--------------------------------------------------------------------|--------------------------------------------------------|-----------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Font     | [Roboto Mono](https://fonts.google.com/specimen/Roboto+Mono)       | [Christian Robertson](http://christianrobertson.com/)  | [Apache 2.0](http://www.apache.org/licenses/)             | Monospaced addition to the Roboto type family. Like the other members of the Roboto family, the fonts are optimized for readability on screens across a wide variety of devices and reading environments.                                                               |
| Textures | [Solar System Planets](https://www.solarsystemscope.com/textures/) | [Solar System Scope](https://www.solarsystemscope.com) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) | Textures in this pack are based on NASA elevation and imagery data. Colors and shades of the textures are tuned accordng to true-color photos made by Messenger, Viking and Cassini spacecrafts, and, of course, the Hubble Space Telescope.                            |
