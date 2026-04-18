# PenStream

Transform your Android tablet or phone into a high-performance graphics tablet with ultra-low latency.

## Features

- **1080p60 streaming** with <10ms latency
- **DirectX 11 capture** for zero-copy screen capture
- **NVENC hardware encoding** for minimal CPU usage
- **Pressure sensitivity** support (4096 levels)
- **Automatic discovery** - no manual IP configuration
- **Cross-platform input** - works with any drawing app

## Architecture

```
┌─────────────────┐         UDP          ┌─────────────────┐
│  Windows Server │◄────────────────────►│  Android Client │
│                 │                      │                 │
│  DXGI Capture   │────┐                 │  MediaCodec     │
│  NVENC Encode   │────┼───── Video ────►│  OpenGL Render  │
│  Input Handler  │◄───┘                 │  Input Capture  │
└─────────────────┘                      └─────────────────┘
```

## Prerequisites

### Server (Windows)
- Windows 10/11 with NVIDIA GPU (for NVENC)
- Visual Studio 2022 with C++ workload
- vcpkg package manager
- NVIDIA Video Codec SDK

### Client (Android)
- Android 8.0 (API 26) or higher
- Android Studio with NDK
- Device with stylus support (recommended)

## Building

### Server
```batch
cd scripts
build_server.bat
```

### Android
```batch
cd scripts
build_android.bat
```

## Installation

1. Run `penstream_server.exe` on your Windows PC
2. Install the APK on your Android device
3. Connect both devices to the same WiFi network
4. Open the app and select your PC from the list

## Configuration

Edit `config.json` on the server:

```json
{
  "port": 9696,
  "width": 1920,
  "height": 1080,
  "fps": 60,
  "bitrate_kbps": 10000,
  "encoder": "nvenc",
  "low_latency": true
}
```

## Troubleshooting

**No servers found**
- Ensure both devices are on the same network
- Check Windows Firewall is not blocking UDP port 9696

**High latency**
- Reduce resolution or bitrate in settings
- Use 5GHz WiFi instead of 2.4GHz
- Ensure NVENC is being used (check server logs)

**No pressure sensitivity**
- Your device must support stylus pressure
- Some apps may not recognize virtual input - try Krita or Clip Studio

## License

MIT License - see LICENSE file for details

## Credits

Built with:
- DirectX 11 Desktop Duplication API
- NVIDIA NVENC
- Android MediaCodec
- OpenGL ES 3.0
