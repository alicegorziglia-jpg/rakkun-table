@echo off
echo Building PenStream Server...

cd /d "%~dp0.."

REM Check for vcpkg
if not defined VCPKG_ROOT (
    echo Error: VCPKG_ROOT not set
    echo Please install vcpkg and set VCPKG_ROOT environment variable
    exit /b 1
)

REM Install dependencies via vcpkg
echo Installing dependencies...
%VCPKG_ROOT%\vcpkg install spdlog:x64-windows nlohmann-json:x64-windows

REM Create build directory
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring...
cmake .. -G "Visual Studio 17 2022" -A x64 ^
    -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
    -DCMAKE_INSTALL_PREFIX=%CD%\install

REM Build
echo Building...
cmake --build . --config Release

echo.
echo Build complete!
echo Executable: build\bin\Release\penstream_server.exe
echo.
echo To run the server:
echo cd build\bin\Release
echo penstream_server.exe
pause
