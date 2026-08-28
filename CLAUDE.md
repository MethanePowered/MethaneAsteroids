# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this repository is

A single cross-platform graphics sample application (`MethaneAsteroids`) built on top of
[MethaneKit](https://github.com/MethanePowered/MethaneKit). Almost all infrastructure — the
application framework, the RHI abstraction over DirectX 12 / Vulkan / Metal, and the CMake
functions used throughout this repo — lives in MethaneKit, which is **fetched at CMake configure
time via CPM**, not vendored. Expect to read MethaneKit sources in the CPM cache to understand
build behavior.

## Build

There are no tests, no test presets, and no linters configured in this repo. `install` is the
normal build target — it is what CI builds and what produces a runnable app layout.

```bash
cmake --list-presets
```

```bash
cmake --preset VS2026-Win64-DX-Default
```

```bash
cmake --build --preset VS2026-Win64-DX-Release --target install --parallel 8
```

Preset names follow `[VS2022|VS2026|Xcode|Make|Ninja]-[Win64|Win32|Win|Lin|Mac|iOS|tvOS]-[Sim]-[DX|VK|MTL]-[Default|Profile]`
(see the presets matrix in [Build/README.md](Build/README.md#cmake-presets)). Each preset gets its
own `Build/Output/<presetName>/{Build,Install}` tree.

Helper wrappers exist for a non-preset build: [Build/Windows/Build.bat](Build/Windows/Build.bat)
and [Build/Unix/Build.sh](Build/Unix/Build.sh) (each documents its flags in a header comment).
CI drives presets through [Build/Unix/CI/CMakeConfigurePreset.sh](Build/Unix/CI/CMakeConfigurePreset.sh)
and [Build/Unix/CI/CMakeBuildPreset.sh](Build/Unix/CI/CMakeBuildPreset.sh).

The built app lands in `Build/Output/<presetName>/Install/Apps/`, alongside `dxcompiler.dll` and
(where produced) the app PDB. The install prefix also receives MethaneKit's own `include/`, `lib/`
and `share/` — that is expected, not a misconfiguration.

### Configurations

`Default` presets build `Debug`/`Release`; `Profile` presets build **`RelWithDebInfo`** with Tracy
profiling, GPU instrumentation and scope timers enabled. "Profile" is a preset family, not a CMake
configuration name — when invoking CMake directly on a Profile build tree, pass
`--config RelWithDebInfo`.

### Configure from a shell with a clean `PATH`

`MethaneShaders.cmake` bakes `$ENV{PATH}` verbatim into the generated shader-compile commands. If
you configure from a shell whose `PATH` contains newlines or other oddities (notably the Git Bash /
MSYS environment available in this session, which prepends path-conversion diagnostics), the
generated command is split across lines and every shader build fails with
`cmake -E env: no command given`. **Run CMake configure from PowerShell on Windows**, and
reconfigure from a clean shell if you see that error.

## Versioning coupling

`ASTEROIDS_VERSION_MAJOR/MINOR/PATCH` in the root [CMakeLists.txt](CMakeLists.txt) do double duty:
[Externals/MethaneKit.cmake](Externals/MethaneKit.cmake) passes them straight to `CPMAddPackage` as
the MethaneKit `VERSION`, so **bumping the patch version changes which MethaneKit tag is fetched**.
The same numbers are duplicated in `env:` of [.github/workflows/ci-build.yml](.github/workflows/ci-build.yml)
(`METHANE_VERSION_*`, injected back via `-D` by the CI configure script) and in the `BUILD_VERSION_*`
variables of both `Build.bat` and `Build.sh`. A version bump must touch all of them.

## Externals and CMake module resolution

[Externals/CMakeLists.txt](Externals/CMakeLists.txt) is included **first** from the root
`CMakeLists.txt`, and MethaneKit must stay first within it: it defines the `MethaneBuildOptions`
target and appends `${MethaneKit_SOURCE_DIR}/CMake` to `CMAKE_MODULE_PATH`. Every
`include(Methane*)` elsewhere in this repo resolves into the fetched MethaneKit checkout.

Because `add_subdirectory(Externals)` comes first, a failure in any external's install rules aborts
the whole install before `App/` is reached — the symptom is a populated `Install/lib` and
`Install/include` with **no `Install/Apps`**. [Externals/FastNoise2.cmake](Externals/FastNoise2.cmake)
carries a workaround for exactly this (FastNoise2 unconditionally installs a PDB directory that only
exists in Debug under MSVC).

CPM sources are cached outside the build tree in `Build/Output/ExternalsCache` by default; the root
`CMakeLists.txt` deliberately skips setting `CPM_SOURCE_CACHE` when `CLION_IDE` is in the
environment, to avoid cache collisions across CLion's parallel configurations.

## Code structure

Three targets, bottom-up:

- **`MethanePerlinNoise`** ([Modules/PerlinNoise](Modules/PerlinNoise)) — thin wrapper over FastNoise2
  used for procedural asteroid textures.
- **`MethaneAsteroidsSimulation`** ([Modules/Simulation](Modules/Simulation)) — the content: `Asteroid`
  (one procedurally generated icosahedron-based mesh + triplanar noise texture array),
  `AsteroidsArray` (an "uber mesh" of up to ~1000 unique meshes with per-instance parameters, LOD
  selection and parallel update via Taskflow), and `Planet`. Also owns the HLSL shaders.
- **`MethaneAsteroids`** ([App](App)) — `AsteroidsApp` derives from MethaneKit's
  `UserInterface::App<AsteroidsFrame>` and implements `Init`/`Update`/`Render`/`Resize`;
  `AsteroidsAppController` maps the sample-specific keys (complexity, LOD, parallel rendering).

Rendering is frame-buffered: `AsteroidsFrame` holds the per-frame command lists (`parallel_cmd_list`
for the asteroid field, `serial_cmd_list`, `final_cmd_list`) and program bindings for sky box, planet
and asteroids.

### Shaders

HLSL under `Modules/Simulation/Shaders/` is compiled at build time by DXC (from a downloaded binary
package) into DXIL or SPIR-V, then embedded into the binary as a CMRC resource library via
`add_methane_shaders_source` / `add_methane_shaders_library`. `Asteroids.hlsl` is compiled once per
`TEXTURES_COUNT` value listed in [Modules/Simulation/CMakeLists.txt](Modules/Simulation/CMakeLists.txt),
producing one entry point variant per texture-array size.

Uniform structs are shared between C++ and HLSL: headers like `Shaders/SceneConstants.h` and
`Shaders/AsteroidUniforms.h` are written in HLSL types and `#include`d from C++ inside a
`namespace hlslpp` under `#pragma pack(push, 16)` (see [App/AsteroidsApp.h](App/AsteroidsApp.h)).
When changing a uniform layout, edit the shared header only — never redeclare the struct on one side.

Textures under `Resources/Textures/` are likewise embedded via `add_methane_embedded_textures`
rather than shipped as loose files.

## Conventions

- C++20 (`CMAKE_CXX_STANDARD 20`), MSVC uses static runtime (`/MT`, `/MTd` in Debug).
- Namespaces are aliased locally as `gfx` (`Methane::Graphics`), `rhi` (`Methane::Graphics::Rhi`),
  `pin` (`Methane::Platform::Input`); sample code lives in `Methane::Samples`.
- `// NOSONAR` comments are intentional SonarQube suppressions carried over from MethaneKit's
  analysis setup — keep them when editing surrounding lines.
