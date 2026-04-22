Development Guide

- Requisitos
  - Compilador C++17+/C++20 compatible
  - CMake
  - GoogleTest (para tests)

- Construcción del servidor (Windows)
  - cmake --build . --config Release
  - cmake --build . --config Debug
  - Ejecutar: ./bin/Release/penstream_server.exe

- Construcción para pruebas (Linux/Windows donde sea posible)
  - mkdir build && cd build
  - cmake .. -DCMAKE_BUILD_TYPE=Debug
  - cmake --build . --config Debug
  - ctest o ./tests/packet_test (según cómo esté configurado)

- Estructura relevante
  - server/src/capture/      -> DXGI Desktop Duplication
  - server/src/encode/       -> NVENC/AMF encoding
  - server/src/network/      -> UDP transport + packet builder
  - server/src/input/        -> Virtual input handling

- Prácticas de calidad
  - RAII en todo, sin raw pointers
  - Sin excepciones en rutas críticas; usar expected o códigos de error
  - Logging estructurado con spdlog
  - Tests con GoogleTest para módulos clave
  - Documentar cambios en protocol_spec.md cuando se modifiquen paquetes
