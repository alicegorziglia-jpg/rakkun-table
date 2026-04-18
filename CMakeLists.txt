cmake_minimum_required(VERSION 3.25)
project(PenStream VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Opciones de build
option(BUILD_SERVER "Build Windows server" ON)
option(BUILD_ANDROID "Build Android client" OFF)

# Configuración global
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Server (Windows)
if(BUILD_SERVER AND WIN32)
    add_subdirectory(server)
endif()

# Android client se builda desde Android Studio con su propio Gradle
