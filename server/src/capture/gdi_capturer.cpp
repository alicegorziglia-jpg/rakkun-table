#include "gdi_capturer.h"
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <chrono>

namespace penstream::capture {

GDICapturer::GDICapturer()
    : m_screen_dc(nullptr)
    , m_mem_dc(nullptr)
    , m_bitmap(nullptr)
    , m_old_bitmap(nullptr)
    , m_width(0)
    , m_height(0)
    , m_initialized(false)
{}

GDICapturer::~GDICapturer() {
    shutdown();
}

bool GDICapturer::initialize() {
    // Obtener dimensiones del escritorio virtual completo
    // (incluye múltiples monitores)
    m_width  = static_cast<uint32_t>(GetSystemMetrics(SM_CXVIRTUALSCREEN));
    m_height = static_cast<uint32_t>(GetSystemMetrics(SM_CYVIRTUALSCREEN));

    if (m_width == 0 || m_height == 0) {
        spdlog::error("[GDI] GetSystemMetrics devolvió dimensiones inválidas: {}x{}",
                      m_width, m_height);
        return false;
    }

    // DC del escritorio
    m_screen_dc = GetDC(nullptr);
    if (!m_screen_dc) {
        spdlog::error("[GDI] GetDC(nullptr) falló");
        return false;
    }

    // DC compatible en memoria
    m_mem_dc = CreateCompatibleDC(m_screen_dc);
    if (!m_mem_dc) {
        spdlog::error("[GDI] CreateCompatibleDC falló");
        ReleaseDC(nullptr, m_screen_dc);
        m_screen_dc = nullptr;
        return false;
    }

    // Bitmap para recibir los píxeles
    m_bitmap = CreateCompatibleBitmap(m_screen_dc, m_width, m_height);
    if (!m_bitmap) {
        spdlog::error("[GDI] CreateCompatibleBitmap falló ({}x{})", m_width, m_height);
        DeleteDC(m_mem_dc);
        ReleaseDC(nullptr, m_screen_dc);
        m_mem_dc    = nullptr;
        m_screen_dc = nullptr;
        return false;
    }

    m_old_bitmap = static_cast<HBITMAP>(SelectObject(m_mem_dc, m_bitmap));

    m_initialized = true;
    spdlog::info("[GDI] Capturer inicializado: {}x{} (modo fallback — funciona en RDP/VM)",
                 m_width, m_height);
    spdlog::warn("[GDI] Nota: la captura GDI tiene más latencia que DXGI (~5-15ms extra). "
                 "Para latencia óptima ejecuta en una sesión de escritorio local con GPU NVIDIA.");
    return true;
}

bool GDICapturer::capture_frame(Frame& out_frame) {
    if (!m_initialized) return false;

    // Copiar escritorio al bitmap en memoria
    int x0 = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y0 = GetSystemMetrics(SM_YVIRTUALSCREEN);

    BOOL ok = BitBlt(
        m_mem_dc,
        0, 0,
        static_cast<int>(m_width),
        static_cast<int>(m_height),
        m_screen_dc,
        x0, y0,
        SRCCOPY | CAPTUREBLT // CAPTUREBLT incluye ventanas en capas (layered windows)
    );

    if (!ok) {
        spdlog::warn("[GDI] BitBlt falló: {}", GetLastError());
        return false;
    }

    // Leer los píxeles del bitmap al buffer de Frame (formato BGRA, 4 bytes/px)
    const uint32_t byte_count = m_width * m_height * 4;
    out_frame.data.resize(byte_count);

    BITMAPINFOHEADER bi = {};
    bi.biSize        = sizeof(bi);
    bi.biWidth       = static_cast<LONG>(m_width);
    bi.biHeight      = -static_cast<LONG>(m_height); // negativo = top-down
    bi.biPlanes      = 1;
    bi.biBitCount    = 32; // BGRA
    bi.biCompression = BI_RGB;

    int lines = GetDIBits(
        m_mem_dc,
        m_bitmap,
        0,
        m_height,
        out_frame.data.data(),
        reinterpret_cast<BITMAPINFO*>(&bi),
        DIB_RGB_COLORS
    );

    if (lines == 0) {
        spdlog::warn("[GDI] GetDIBits devolvió 0 líneas");
        return false;
    }

    out_frame.width      = m_width;
    out_frame.height     = m_height;
    out_frame.is_keyframe = false;

    // Timestamp en microsegundos desde epoch
    using namespace std::chrono;
    out_frame.timestamp_us = static_cast<uint64_t>(
        duration_cast<microseconds>(
            system_clock::now().time_since_epoch()
        ).count()
    );

    return true;
}

void GDICapturer::shutdown() {
    if (m_mem_dc && m_old_bitmap) {
        SelectObject(m_mem_dc, m_old_bitmap);
        m_old_bitmap = nullptr;
    }
    if (m_bitmap) {
        DeleteObject(m_bitmap);
        m_bitmap = nullptr;
    }
    if (m_mem_dc) {
        DeleteDC(m_mem_dc);
        m_mem_dc = nullptr;
    }
    if (m_screen_dc) {
        ReleaseDC(nullptr, m_screen_dc);
        m_screen_dc = nullptr;
    }
    m_initialized = false;
    spdlog::info("[GDI] Capturer cerrado.");
}

} // namespace penstream::capture
