cmake_minimum_required(VERSION 3.25)
project(PenStreamServer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(DirectXMath REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Executable
add_executable(penstream_server
    src/main.cpp
    src/capture/dxgi_capturer.cpp
    src/capture/frame_pool.cpp
    src/encode/nvenc_encoder.cpp
    src/encode/amf_encoder.cpp
    src/network/webrtc_server.cpp
    src/network/udp_transport.cpp
    src/network/packet_builder.cpp
    src/input/input_handler.cpp
    src/input/virtual_input.cpp
    src/config/config_loader.cpp
)

target_include_directories(penstream_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps
)

target_link_libraries(penstream_server PRIVATE
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    d3d11
    dxgi
    user32
)

# NVENC SDK
if(DEFINED ENV{NVENC_SDK_PATH})
    target_include_directories(penstream_server PRIVATE $ENV{NVENC_SDK_PATH}/Interfaces)
    target_link_directories(penstream_server PRIVATE $ENV{NVENC_SDK_PATH}/Lib/x64)
    target_link_libraries(penstream_server PRIVATE nvencodeapi)
endif()

# Windows specific
target_compile_definitions(penstream_server PRIVATE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

# Install
install(TARGETS penstream_server RUNTIME DESTINATION bin)
