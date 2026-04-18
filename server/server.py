"""
GraphicTablet Server - Windows Host
Captura la pantalla con DXGI (GPU directo) y la transmite al Android via UDP.
"""

import subprocess
import socket
import sys
import json
import os
from discovery import discover_android_interactive, get_local_ip


# ─────────────────────────────────────────────
# Detección de encoder de hardware
# ─────────────────────────────────────────────

ENCODERS = [
    ("h264_nvenc", "NVIDIA (NVENC)"),
    ("h264_amf",   "AMD (AMF)"),
    ("h264_qsv",   "Intel (QuickSync)"),
    ("libx264",    "Software CPU (fallback)"),
]

def detect_encoder():
    """Prueba encoders en orden, retorna el primero disponible."""
    for codec, name in ENCODERS:
        try:
            result = subprocess.run(
                [
                    "ffmpeg", "-hide_banner",
                    "-f", "lavfi", "-i", "nullsrc=s=64x64:r=1",
                    "-vframes", "1",
                    "-c:v", codec,
                    "-f", "null", "-"
                ],
                capture_output=True,
                timeout=8
            )
            if result.returncode == 0:
                return codec, name
        except (subprocess.TimeoutExpired, FileNotFoundError):
            continue
    return None, None


# ─────────────────────────────────────────────
# Construcción del comando FFmpeg
# ─────────────────────────────────────────────

def encoder_params(encoder):
    """Parámetros óptimos de baja latencia por encoder."""
    if encoder == "h264_nvenc":
        return [
            "-preset", "p1",          # más rápido
            "-tune", "ull",            # ultra low latency
            "-zerolatency", "1",
            "-rc", "cbr",
            "-bf", "0",                # sin B-frames (reduce latencia)
        ]
    elif encoder == "h264_amf":
        return [
            "-quality", "speed",
            "-usage", "ultralowlatency",
            "-bf", "0",
        ]
    elif encoder == "h264_qsv":
        return [
            "-preset", "veryfast",
            "-low_power", "1",
            "-bf", "0",
        ]
    else:  # libx264 fallback
        return [
            "-preset", "ultrafast",
            "-tune", "zerolatency",
        ]


def build_cmd(client_ip, port, encoder, fps=60, monitor=0, bitrate="8M"):
    """
    Construye el comando FFmpeg completo.
    
    - ddagrab: captura DXGI directo desde GPU (como OBS)
    - hwdownload + format=bgra: baja el frame a RAM para encoders sin soporte d3d11 directo
    - Para NVENC con CUDA se puede optimizar más (ver comentario abajo)
    """

    # Para NVENC puro sin bajar a CPU, usaría:
    # '-init_hw_device', 'd3d11va=hw',
    # '-init_hw_device', 'cuda=cu@hw',  
    # '-filter_complex', f'ddagrab={monitor},hwmap=derive_device=cuda,format=cuda',
    # Pero la versión con hwdownload es más compatible entre GPUs.

    filter_graph = f"ddagrab=output_idx={monitor},hwdownload,format=bgra"

    cmd = [
        "ffmpeg", "-hide_banner", "-loglevel", "warning",

        # Inicializar dispositivo D3D11
        "-init_hw_device", "d3d11va=hw",

        # Captura DXGI vía ddagrab
        "-filter_complex", filter_graph,
        "-framerate", str(fps),

        # Encoder
        "-c:v", encoder,
        *encoder_params(encoder),

        # Parámetros de stream
        "-g", str(fps * 2),       # keyframe cada 2 segundos
        "-b:v", bitrate,
        "-maxrate", bitrate,
        "-bufsize", "500k",       # buffer chico = menos latencia

        # Output: MPEG-TS sobre UDP
        "-f", "mpegts",
        f"udp://{client_ip}:{port}?pkt_size=1316&buffer_size=65535"
    ]
    return cmd


# ─────────────────────────────────────────────
# Configuración guardada
# ─────────────────────────────────────────────

CONFIG_FILE = os.path.join(os.path.dirname(__file__), "config.json")

def load_config():
    if os.path.exists(CONFIG_FILE):
        with open(CONFIG_FILE) as f:
            return json.load(f)
    return {}

def save_config(cfg):
    with open(CONFIG_FILE, "w") as f:
        json.dump(cfg, f, indent=2)


# ─────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────

def main():
    print("╔══════════════════════════════════╗")
    print("║     GraphicTablet Server v0.1    ║")
    print("╚══════════════════════════════════╝\n")

    local_ip = get_local_ip()
    print(f"  Tu IP local: {local_ip}")
    print(f"  (Asegurate que tu Android esté en la misma red WiFi)\n")

    cfg = load_config()

    # Auto-discovery del Android (ya no hay que ingresar IP manualmente)
    client_ip, discovered_port = discover_android_interactive()
    if not client_ip:
        print("ERROR: No se encontró ningún Android en la red. ¿Está la app abierta?")
        sys.exit(1)

    # Puerto (usa el que el Android reportó, o el default)
    default_port = str(discovered_port or cfg.get("port", "9000"))
    port = input(f"Puerto [{default_port}]: ").strip() or default_port

    # FPS
    default_fps = cfg.get("fps", "60")
    fps = input(f"FPS [{default_fps}]: ").strip() or default_fps

    # Monitor
    default_monitor = cfg.get("monitor", "0")
    monitor = input(f"Monitor (0 = principal) [{default_monitor}]: ").strip() or default_monitor

    # Bitrate
    default_bitrate = cfg.get("bitrate", "8M")
    bitrate = input(f"Bitrate [{default_bitrate}]: ").strip() or default_bitrate

    # Guardar config
    save_config({
        "client_ip": client_ip,
        "port": port,
        "fps": fps,
        "monitor": monitor,
        "bitrate": bitrate
    })

    # Detectar encoder
    print("\n  Detectando encoder de hardware...")
    encoder, name = detect_encoder()
    if not encoder:
        print("ERROR: No se encontró FFmpeg. Instalalo desde https://ffmpeg.org/download.html")
        sys.exit(1)
    print(f"  ✓ Usando: {name}\n")

    # Construir y mostrar comando
    cmd = build_cmd(client_ip, int(port), encoder, int(fps), int(monitor), bitrate)
    print("  Comando FFmpeg:")
    print("  " + " ".join(cmd) + "\n")

    print(f"  ✓ Transmitiendo a {client_ip}:{port} @ {fps}fps")
    print("  Abrí la app en tu Android y conectate.")
    print("  Presioná Ctrl+C para detener.\n")
    print("─" * 40)

    try:
        subprocess.run(cmd)
    except KeyboardInterrupt:
        print("\n\n  Stream detenido.")
    except FileNotFoundError:
        print("\nERROR: FFmpeg no encontrado en PATH.")
        print("Descargalo desde: https://ffmpeg.org/download.html")


if __name__ == "__main__":
    main()
