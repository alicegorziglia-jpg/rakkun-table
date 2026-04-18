# GraphicTablet

> Tablet gráfica de baja latencia: usa tu Android como pantalla táctil/stylus del PC.  
> 60fps · Resolución nativa · Hardware encoding (NVENC/AMF/QSV)

---

## Estado del proyecto

| Fase | Descripción | Estado |
|---|---|---|
| **1** | Stream Windows → Android (video 60fps) | ✅ En desarrollo |
| **2** | Input Android → Windows (touch/stylus con presión) | 🔜 Próximo |
| **3** | Windows Ink API (presión real en apps de dibujo) | 🔜 Planificado |

---

## Estructura

```
GraphicTablet/
├── server/           # Python - Windows host
│   ├── server.py     # Captura + stream con FFmpeg
│   └── README.md     # Instrucciones
└── android/          # Kotlin - cliente Android
    └── app/src/main/
        ├── java/com/graphictablet/
        │   └── MainActivity.kt    # Player + input sender
        └── res/layout/
            └── activity_main.xml
```

---

## Setup rápido

### 1. Windows (server)
```bash
# Instalar FFmpeg
winget install ffmpeg

# Correr el server
cd server
python server.py
```

### 2. Android (cliente)
1. Abrir el proyecto `android/` en Android Studio
2. Build → Run en tu dispositivo
3. Ingresar la IP del Windows en el server cuando te lo pida

---

## Por qué es más rápido que Weylus

| | Weylus | GraphicTablet |
|---|---|---|
| Captura | Software (CPU) | DXGI GPU directo |
| Encoding | libx264 software | NVENC/AMF hardware |
| Transporte | WebRTC (~100ms) | UDP MPEG-TS (~15ms) |
| Decoder Android | WebRTC (JS) | MediaCodec hardware |
| Input | uinput (Linux only) | Windows Ink API |
