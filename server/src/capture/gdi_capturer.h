#pragma once

#include <windows.h>
#include <vector>
#include <cstdint>
#include "dxgi_capturer.h" // reutilizamos el struct Frame

namespace penstream::capture {

// Capturer de fallback usando GDI (BitBlt).
// Funciona en sesiones RDP, VMs sin GPU passthrough, y cualquier
// escritorio de Windows donde DXGI Desktop Duplication falle.
// Latencia algo mayor que DXGI (~5-15ms extra), pero completamente funcional.
class GDICapturer {
public:
    GDICapturer();
    ~GDICapturer();

    bool initialize();
    bool capture_frame(Frame& out_frame);
    void shutdown();

    uint32_t get_width()  const { return m_width; }
    uint32_t get_height() const { return m_height; }

private:
    HDC     m_screen_dc;
    HDC     m_mem_dc;
    HBITMAP m_bitmap;
    HBITMAP m_old_bitmap;

    uint32_t m_width;
    uint32_t m_height;
    bool     m_initialized;
};

} // namespace penstream::capture
