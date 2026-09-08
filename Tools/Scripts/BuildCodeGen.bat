@echo off

echo "Running BuildCodeGen.bat from %CD%"

mkdir .\Build\CodeGen
pushd .\Build\CodeGen

choice /C YN /T 3 /D N /M "Regenerate CMake? (will continue without regenerating in 3s)"
if errorlevel 2 goto skip_cmake_gen

rem On ARM64 devices (Windows on ARM), the Visual Studio generator defaults
rem to the x64 platform which would build the codegen tool for emulated x64.
rem Force the native ARM64 platform when running natively on an ARM64 host.
set "CMAKE_PLATFORM_ARGS="
if /I "%PROCESSOR_ARCHITECTURE%"=="ARM64" set "CMAKE_PLATFORM_ARGS=-A ARM64"
cmake ..\..\Tools\CodeGen %CMAKE_PLATFORM_ARGS%

:skip_cmake_gen
cmake --build . --target hyperion-codegen --parallel 4
if errorlevel 1 (
    exit /b 1
)

set MOVED_EXE=0
set MOVED_DLL=0

if exist hyperion-codegen.exe (
    echo Found hyperion-codegen.exe in current directory
    set MOVED_EXE=1
) else (
    if exist Debug\hyperion-codegen.exe (
        echo Found hyperion-codegen.exe in Debug directory
        move Debug\hyperion-codegen.exe ..
        set MOVED_EXE=1
    ) else (
        if exist Release\hyperion-codegen.exe (
            echo Found hyperion-codegen.exe in Release directory
            move Release\hyperion-codegen.exe ..
            set MOVED_EXE=1
        ) else (
            echo Could not find hyperion-codegen.exe executable!
            exit /b 1
        )
    )
)

popd
