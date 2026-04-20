#pragma once

#include "encoder_interface.h"
#include "capture/dxgi_capturer.h"
#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mftransform.h>
#include <vector>
#include <cstdint>

namespace penstream::encode {

// Software H.264 encoder using Windows Media Foundation (built-in on Windows 8+).
// No external SDK required. Used as fallback when NVENC/AMF are unavailable.
class MFEncoder {
public:
    MFEncoder();
    ~MFEncoder();

    bool initialize(const EncodeConfig& config);
    bool encode(const capture::Frame& frame, std::vector<uint8_t>& out_data);
    void shutdown();
    const char* name() const { return "MF-Software-H264"; }

private:
    bool create_transform(const EncodeConfig& config);
    bool set_input_type(uint32_t width, uint32_t height, uint32_t fps);
    bool set_output_type(uint32_t width, uint32_t height, uint32_t bitrate_bps, uint32_t fps);
    void bgra_to_nv12(const uint8_t* bgra, uint32_t width, uint32_t height,
                      uint8_t* nv12, uint32_t nv12_stride);

    IMFTransform*   m_transform;
    uint32_t        m_width;
    uint32_t        m_height;
    uint32_t        m_fps;
    uint32_t        m_bitrate_bps;
    uint64_t        m_frame_num;
    bool            m_mf_started;
    bool            m_initialized;
};

} // namespace penstream::encode
