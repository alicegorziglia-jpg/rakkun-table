# GraphicTablet - Server (Windows)

## Requisitos

1. **Python 3.8+** — https://python.org
2. **FFmpeg 6.0+** (con soporte `ddagrab`) — https://ffmpeg.org/download.html
   - Recomendado: build de **gyan.dev** (incluye encoders NVENC/AMF/QSV)
   - Agregar FFmpeg al PATH de Windows

## Instalación rápida de FFmpeg (PowerShell como Admin)

```powershell
winget install ffmpeg
```

O manual: descargá el .zip de https://www.gyan.dev/ffmpeg/builds/ → `ffmpeg-release-full.7z`

## Uso

```bash
python server.py
```

Seguí las instrucciones en pantalla:
1. Ingresá la IP del Android (la ves en Ajustes → WiFi → tu red)
2. Dejá el puerto en 9000 (o cambialo si ya está en uso)
3. Elegí FPS (60 recomendado, 30 si el WiFi es lento)
4. Abrí la app en el Android

## Configuración guardada

La config se guarda en `config.json` — la próxima vez no tenés que re-ingresar nada.

## Troubleshooting

| Error | Solución |
|---|---|
| `ddagrab` not found | Actualizá FFmpeg a versión 5.1+ |
| No encoder found | Instalá FFmpeg con builds `full` (incluye NVENC/AMF) |
| Pantalla negra en Android | Verificá que ambos dispositivos estén en la misma red |
| Alta latencia | Reducí el bitrate o usá WiFi 5GHz |

## Encoders por GPU

| GPU | Encoder usado |
|---|---|
| NVIDIA | h264_nvenc (NVENC) |
| AMD | h264_amf |
| Intel | h264_qsv |
| Sin GPU / fallback | libx264 (CPU, más lento) |
