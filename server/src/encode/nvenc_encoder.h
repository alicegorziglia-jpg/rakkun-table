#pragma once

#include "encoder_interface.h"
#include <windows.h>
#include <vector>
#include <d3d11.h>

// NVENC forward declarations
// Full types from: https://developer.nvidia.com/nvidia-video-codec-sdk
#ifndef NV_ENCODE_API_FUNCTION_LIST
typedef struct _NV_ENCODE_API_FUNCTION_LIST NV_ENCODE_API_FUNCTION_LIST;
#endif

namespace penstream::encode {

class NVEncoder : public IEncoder {
public:
    NVEncoder();
    ~NVEncoder() override;

    bool initialize(const EncodeConfig& config) override;
    bool encode(const capture::Frame& frame, std::vector<uint8_t>& out_data) override;
    void shutdown() override;
    const char* name() const override { return "NVENC"; }

    // Set D3D11 device for interop (called after DXGI init)
    void set_d3d_device(ID3D11Device* device);

private:
    bool load_nvenc();
    bool create_encoder(const EncodeConfig& config);
    bool allocate_input_buffer();

    HMODULE m_nvenc_lib;
    NV_ENCODE_API_FUNCTION_LIST* m_nvenc;
    void* m_encoder;
    ID3D11Device* m_d3d_device;
    ID3D11Texture2D* m_input_texture;
    void* m_mapped_buffer;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_fps;
    bool m_initialized;
};

} // namespace penstream::encode
