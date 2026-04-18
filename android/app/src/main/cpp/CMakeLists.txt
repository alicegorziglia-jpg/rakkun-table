cmake_minimum_required(VERSION 3.25)
project(penstream_android)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add Android log library
find_library(log-lib log)

# Native library
add_library(penstream SHARED
    jni_interface.cpp
    video_decoder.cpp
    renderer.cpp
    input_capture.cpp
    network_client.cpp
)

# Include directories
target_include_directories(penstream PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# Link libraries
target_link_libraries(penstream
    android
    ${log-lib}
    EGL
    GLESv3
)

# Enable optimizations
if(CMAKE_BUILD_TYPE Release)
    target_compile_options(penstream PRIVATE -O3)
endif()
