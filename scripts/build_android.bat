@echo off
echo Building PenStream Android APK...

cd /d "%~dp0..\android"

REM Check for Android SDK
if not defined ANDROID_SDK_ROOT (
    echo Error: ANDROID_SDK_ROOT not set
    exit /b 1
)

REM Check for Android NDK
if not defined ANDROID_NDK_ROOT (
    echo Error: ANDROID_NDK_ROOT not set
    exit /b 1
)

REM Clean previous build
call gradlew.bat clean

REM Build debug APK
echo Building debug APK...
call gradlew.bat assembleDebug

if exist app\build\outputs\apk\debug\app-debug.apk (
    echo.
    echo ========================================
    echo Debug APK built successfully!
    echo ========================================
    echo Location: android\app\build\outputs\apk\debug\app-debug.apk
    echo.
    echo To install on device via USB:
    echo   adb install app\build\outputs\apk\debug\app-debug.apk
    echo.
    echo Or transfer the APK to your device and install manually
) else (
    echo.
    echo Build failed! Check the error messages above.
    exit /b 1
)
pause
