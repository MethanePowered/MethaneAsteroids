@REM Run 'Build.bat' with optional arguments:
@REM   --vs2022   - build with Visual Studio 2022 instead of Visual Studio 2019 by default
@REM   --win32    - 32-bit build instead of 64-bit by default
@REM   --debug    - Debug build instead of Release build by default
@REM   --vulkan   - use Vulkan graphics API instead of DirectX 12 by default
@REM   --graphviz - enable GraphViz cmake module diagrams generation in Dot and Png formats
@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

SET BUILD_VERSION_MAJOR=0
SET BUILD_VERSION_MINOR=7
SET BUILD_VERSION=%BUILD_VERSION_MAJOR%.%BUILD_VERSION_MINOR%

SET OUTPUT_DIR=%~dp0..\Output
SET SOURCE_DIR=%~dp0..\..
SET START_DIR=%cd%

REM Parse command line options
:options_loop
IF NOT "%1"=="" (
    IF "%1"=="--vs2022" (
        SET USE_VS2022=1
    )
    IF "%1"=="--win32" (
        SET WIN32_BUILD=1
    )
    IF "%1"=="--debug" (
        SET DEBUG_BUILD=1
    )
    IF "%1"=="--vulkan" (
        SET VULKAN_API_ENABLED=ON
    )
    IF "%1"=="--graphviz" (
        SET GRAPHVIZ_ENABLED=1
    )
    SHIFT
    GOTO :options_loop
)

IF DEFINED VULKAN_API_ENABLED (
    SET GFX_API_NAME=Vulkan
    SET GFX_API=VK
) ELSE (
    SET VULKAN_API_ENABLED=OFF
    SET GFX_API_NAME=DirectX 12
    SET GFX_API=DX
)

IF DEFINED WIN32_BUILD (
    SET ARCH_TYPE=Win32
) ELSE (
    SET ARCH_TYPE=x64
)

IF DEFINED DEBUG_BUILD (
    SET BUILD_TYPE=Debug
) ELSE (
    SET BUILD_TYPE=Release
)

IF DEFINED USE_VS2022 (
    SET CMAKE_GENERATOR=Visual Studio 17 2022
) ELSE (
    SET CMAKE_GENERATOR=Visual Studio 16 2019
)

SET CONFIG_DIR=%OUTPUT_DIR%\VisualStudio\%ARCH_TYPE%-MSVC-%GFX_API%-%BUILD_TYPE%-SLN
SET INSTALL_DIR=%CONFIG_DIR%\Install

SET CMAKE_FLAGS= ^
    -A %ARCH_TYPE% ^
    -DASTEROIDS_VERSION_MAJOR=%BUILD_VERSION_MAJOR% ^
    -DASTEROIDS_VERSION_MINOR=%BUILD_VERSION_MINOR% ^
    -DMETHANE_GFX_VULKAN_ENABLED:BOOL=%VULKAN_API_ENABLED% ^
    -DMETHANE_SHADERS_CODEVIEW_ENABLED:BOOL=ON ^
    -DMETHANE_RHI_PIMPL_INLINE_ENABLED:BOOL=ON ^
    -DMETHANE_PRECOMPILED_HEADERS_ENABLED:BOOL=ON ^
    -DMETHANE_RUN_TESTS_DURING_BUILD:BOOL=OFF ^
    -DMETHANE_COMMAND_DEBUG_GROUPS_ENABLED:BOOL=ON ^
    -DMETHANE_LOGGING_ENABLED:BOOL=OFF ^
    -DMETHANE_OPEN_IMAGE_IO_ENABLED:BOOL=OFF ^
    -DMETHANE_SCOPE_TIMERS_ENABLED:BOOL=OFF ^
    -DMETHANE_ITT_INSTRUMENTATION_ENABLED:BOOL=ON ^
    -DMETHANE_ITT_METADATA_ENABLED:BOOL=OFF ^
    -DMETHANE_GPU_INSTRUMENTATION_ENABLED:BOOL=OFF ^
    -DMETHANE_TRACY_PROFILING_ENABLED:BOOL=OFF ^
    -DMETHANE_TRACY_PROFILING_ON_DEMAND:BOOL=OFF

IF DEFINED GRAPHVIZ_ENABLED (
    SET GRAPHVIZ_DIR=%CONFIG_DIR%\GraphViz
    SET GRAPHVIZ_DOT_DIR=!GRAPHVIZ_DIR!\dot
    SET GRAPHVIZ_IMG_DIR=!GRAPHVIZ_DIR!\img
    SET GRAPHVIZ_FILE=MethaneKit.dot
    SET GRAPHVIZ_DOT_EXE=dot.exe
    SET CMAKE_FLAGS=%CMAKE_FLAGS% --graphviz="!GRAPHVIZ_DOT_DIR!\!GRAPHVIZ_FILE!"
)

SET BUILD_DIR=%CONFIG_DIR%\Build

ECHO =========================================================
ECHO Clean build and install Methane %GFX_API_NAME% %ARCH_TYPE% %BUILD_TYPE%
ECHO =========================================================
ECHO  * Build in:   '!BUILD_DIR!'
ECHO  * Install to: '%INSTALL_DIR%'
IF DEFINED GRAPHVIZ_ENABLED (
    ECHO  * Graphviz in: '%GRAPHVIZ_DIR%'
)
ECHO =========================================================

RD /S /Q "%CONFIG_DIR%"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

MKDIR "%BUILD_DIR%"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

CD "%BUILD_DIR%"

ECHO Generating build files for %CMAKE_GENERATOR%...
cmake -G "%CMAKE_GENERATOR%" -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% %CMAKE_FLAGS% "%SOURCE_DIR%"
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

IF DEFINED GRAPHVIZ_ENABLED (
    ECHO ----------
    ECHO Locating GraphViz dot converter...
    where %GRAPHVIZ_DOT_EXE%
    IF %ERRORLEVEL% EQU 0 (
        ECHO Converting GraphViz diagram to image...
        MKDIR "%GRAPHVIZ_IMG_DIR%"
        IF %ERRORLEVEL% NEQ 0 GOTO ERROR
        FOR %%f in ("%GRAPHVIZ_DOT_DIR%\*.*") do (
            ECHO Writing image "%GRAPHVIZ_IMG_DIR%\%%~nxf.png"
            "%GRAPHVIZ_DOT_EXE%" -Tpng "%%f" -o "%GRAPHVIZ_IMG_DIR%\%%~nxf.png"
            IF %ERRORLEVEL% NEQ 0 GOTO ERROR
        )
    ) ELSE (
        ECHO "GraphViz `dot` executable was not found. Skipping graph images generation."
    )
)

ECHO ----------
ECHO Building with %CMAKE_GENERATOR%...
cmake --build . --config %BUILD_TYPE% --target install --parallel
IF %ERRORLEVEL% NEQ 0 GOTO ERROR

GOTO STOP

:ERROR
ECHO Error occurred %ERRORLEVEL%. Script execution was stopped.
GOTO STOP

:STOP
CD "%START_DIR%"
ENDLOCAL
ECHO ON