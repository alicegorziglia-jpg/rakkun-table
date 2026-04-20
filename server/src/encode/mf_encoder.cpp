#include "mf_encoder.h"
#include <spdlog/spdlog.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mftransform.h>
#include <mferror.h>
#include <codecapi.h>
#include <wmcodecdsp.h>
#include <comdef.h>

#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")

namespace penstream::encode {

// Helper: release COM pointer safely
template<typename T>
static void safe_release(T*& p) {
    if (p) { p->Release(); p = nullptr; }
}

MFEncoder::MFEncoder()
    : m_transform(nullptr)
    , m_width(0), m_height(0), m_fps(0), m_bitrate_bps(0)
    , m_frame_num(0)
    , m_mf_started(false)
    , m_initialized(false)
{}

MFEncoder::~MFEncoder() {
    shutdown();
}

bool MFEncoder::initialize(const EncodeConfig& config) {
    HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
    if (FAILED(hr)) {
        spdlog::error("MFStartup failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }
    m_mf_started = true;

    m_width       = config.width;
    m_height      = config.height;
    m_fps         = config.fps;
    m_bitrate_bps = config.bitrate_bps;

    if (!create_transform(config)) {
        return false;
    }

    m_initialized = true;
    spdlog::info("MF software H.264 encoder initialized: {}x{}@{}fps, {}kbps",
                 config.width, config.height, config.fps, config.bitrate_bps / 1000);
    return true;
}

bool MFEncoder::create_transform(const EncodeConfig& config) {
    // Find the H.264 encoder MFT
    IMFActivate** activate   = nullptr;
    UINT32        num_active = 0;

    MFT_REGISTER_TYPE_INFO output_info = {
        MFMediaType_Video, MFVideoFormat_H264
    };

    HRESULT hr = MFTEnumEx(
        MFT_CATEGORY_VIDEO_ENCODER,
        MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER,
        nullptr,       // any input type
        &output_info,  // H.264 output
        &activate,
        &num_active
    );

    if (FAILED(hr) || num_active == 0) {
        spdlog::error("No H.264 MFT encoder found (hr=0x{:08X}, count={})",
                      static_cast<uint32_t>(hr), num_active);
        return false;
    }

    // Activate the first encoder found
    hr = activate[0]->ActivateObject(__uuidof(IMFTransform),
                                     reinterpret_cast<void**>(&m_transform));
    for (UINT32 i = 0; i < num_active; ++i) activate[i]->Release();
    CoTaskMemFree(activate);

    if (FAILED(hr)) {
        spdlog::error("ActivateObject (H.264 MFT) failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    // Configure low-latency via ICodecAPI if available
    ICodecAPI* codec_api = nullptr;
    if (SUCCEEDED(m_transform->QueryInterface(__uuidof(ICodecAPI),
                                              reinterpret_cast<void**>(&codec_api)))) {
        VARIANT v;
        v.vt = VT_BOOL;
        v.boolVal = VARIANT_TRUE;
        codec_api->SetValue(&CODECAPI_AVLowLatencyMode, &v);
        codec_api->Release();
    }

    if (!set_output_type(config.width, config.height, config.bitrate_bps, config.fps)) {
        return false;
    }
    if (!set_input_type(config.width, config.height, config.fps)) {
        return false;
    }

    hr = m_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    if (FAILED(hr)) {
        spdlog::warn("MFT BEGIN_STREAMING failed: 0x{:08X}", static_cast<uint32_t>(hr));
    }

    hr = m_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
    if (FAILED(hr)) {
        spdlog::warn("MFT START_OF_STREAM failed: 0x{:08X}", static_cast<uint32_t>(hr));
    }

    return true;
}

bool MFEncoder::set_output_type(uint32_t width, uint32_t height,
                                uint32_t bitrate_bps, uint32_t fps) {
    IMFMediaType* type = nullptr;
    HRESULT hr = MFCreateMediaType(&type);
    if (FAILED(hr)) return false;

    type->SetGUID(MF_MT_MAJOR_TYPE,  MFMediaType_Video);
    type->SetGUID(MF_MT_SUBTYPE,     MFVideoFormat_H264);
    type->SetUINT32(MF_MT_AVG_BITRATE, bitrate_bps);
    type->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
    MFSetAttributeRatio(type, MF_MT_FRAME_RATE, fps, 1);
    MFSetAttributeRatio(type, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
    type->SetUINT32(MF_MT_MPEG2_PROFILE, eAVEncH264VProfile_High);

    hr = m_transform->SetOutputType(0, type, 0);
    type->Release();

    if (FAILED(hr)) {
        spdlog::error("SetOutputType (H264) failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }
    return true;
}

bool MFEncoder::set_input_type(uint32_t width, uint32_t height, uint32_t fps) {
    // Try NV12 first (native for most encoders), then I420, then IYUV
    const GUID formats[] = { MFVideoFormat_NV12, MFVideoFormat_I420, MFVideoFormat_IYUV };

    for (const GUID& fmt : formats) {
        IMFMediaType* type = nullptr;
        MFCreateMediaType(&type);
        type->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        type->SetGUID(MF_MT_SUBTYPE,    fmt);
        type->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
        MFSetAttributeSize(type, MF_MT_FRAME_SIZE, width, height);
        MFSetAttributeRatio(type, MF_MT_FRAME_RATE, fps, 1);
        MFSetAttributeRatio(type, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);

        HRESULT hr = m_transform->SetInputType(0, type, 0);
        type->Release();
        if (SUCCEEDED(hr)) {
            spdlog::info("MF encoder input format: {}",
                         (fmt == MFVideoFormat_NV12) ? "NV12" :
                         (fmt == MFVideoFormat_I420) ? "I420" : "IYUV");
            return true;
        }
    }

    spdlog::error("No supported input format accepted by H.264 MFT");
    return false;
}

// Convert BGRA (from DXGI) → NV12 (required by most H.264 MFTs)
void MFEncoder::bgra_to_nv12(const uint8_t* bgra, uint32_t width, uint32_t height,
                              uint8_t* nv12, uint32_t stride) {
    // Y plane
    for (uint32_t y = 0; y < height; ++y) {
        const uint8_t* src = bgra + y * width * 4;
        uint8_t*       dst = nv12 + y * stride;
        for (uint32_t x = 0; x < width; ++x) {
            uint8_t b = src[x*4+0];
            uint8_t g = src[x*4+1];
            uint8_t r = src[x*4+2];
            dst[x] = static_cast<uint8_t>(
                ((66*r + 129*g + 25*b + 128) >> 8) + 16);
        }
    }
    // UV plane (interleaved, half resolution)
    uint8_t* uv = nv12 + height * stride;
    for (uint32_t y = 0; y < height/2; ++y) {
        const uint8_t* src0 = bgra + (y*2)   * width * 4;
        const uint8_t* src1 = bgra + (y*2+1) * width * 4;
        uint8_t*       dst  = uv   + y * stride;
        for (uint32_t x = 0; x < width/2; ++x) {
            // Average 2x2 block
            uint32_t b = (src0[x*8+0] + src0[x*8+4] + src1[x*8+0] + src1[x*8+4]) / 4;
            uint32_t g = (src0[x*8+1] + src0[x*8+5] + src1[x*8+1] + src1[x*8+5]) / 4;
            uint32_t r = (src0[x*8+2] + src0[x*8+6] + src1[x*8+2] + src1[x*8+6]) / 4;
            dst[x*2+0] = static_cast<uint8_t>(((-38*r - 74*g + 112*b + 128) >> 8) + 128); // U
            dst[x*2+1] = static_cast<uint8_t>(((112*r - 94*g -  18*b + 128) >> 8) + 128); // V
        }
    }
}

bool MFEncoder::encode(const capture::Frame& frame, std::vector<uint8_t>& out_data) {
    if (!m_initialized || frame.data.empty()) return false;

    const uint32_t stride   = m_width;                         // NV12 Y stride = width
    const uint32_t nv12_sz  = stride * m_height * 3 / 2;      // Y + UV planes

    // --- Build input sample ---
    IMFSample*      sample  = nullptr;
    IMFMediaBuffer* buffer  = nullptr;

    MFCreateSample(&sample);
    MFCreateMemoryBuffer(nv12_sz, &buffer);

    BYTE* buf_ptr = nullptr;
    DWORD max_len = 0;
    buffer->Lock(&buf_ptr, &max_len, nullptr);
    bgra_to_nv12(frame.data.data(), m_width, m_height,
                 reinterpret_cast<uint8_t*>(buf_ptr), stride);
    buffer->Unlock();
    buffer->SetCurrentLength(nv12_sz);

    sample->AddBuffer(buffer);
    buffer->Release();

    // Timestamp and duration in 100ns units
    LONGLONG ts       = static_cast<LONGLONG>(m_frame_num) * 10'000'000LL / m_fps;
    LONGLONG duration = 10'000'000LL / m_fps;
    sample->SetSampleTime(ts);
    sample->SetSampleDuration(duration);

    HRESULT hr = m_transform->ProcessInput(0, sample, 0);
    sample->Release();

    if (FAILED(hr)) {
        spdlog::warn("ProcessInput failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }
    ++m_frame_num;

    // --- Drain output ---
    out_data.clear();

    MFT_OUTPUT_DATA_BUFFER output_data = {};
    output_data.dwStreamID = 0;

    while (true) {
        DWORD status = 0;
        hr = m_transform->ProcessOutput(0, 1, &output_data, &status);

        if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT) {
            break; // Normal: encoder is buffering, will output later
        }

        if (FAILED(hr)) {
            spdlog::warn("ProcessOutput failed: 0x{:08X}", static_cast<uint32_t>(hr));
            break;
        }

        if (output_data.pSample) {
            // Collect all buffers in the output sample
            DWORD buf_count = 0;
            output_data.pSample->GetBufferCount(&buf_count);
            for (DWORD i = 0; i < buf_count; ++i) {
                IMFMediaBuffer* out_buf = nullptr;
                output_data.pSample->GetBufferByIndex(i, &out_buf);
                BYTE*  data     = nullptr;
                DWORD  cur_len  = 0;
                out_buf->Lock(&data, nullptr, &cur_len);
                size_t prev = out_data.size();
                out_data.resize(prev + cur_len);
                memcpy(out_data.data() + prev, data, cur_len);
                out_buf->Unlock();
                out_buf->Release();
            }
            output_data.pSample->Release();
            output_data.pSample = nullptr;
        }
    }

    return !out_data.empty();
}

void MFEncoder::shutdown() {
    if (m_transform) {
        m_transform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
        m_transform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
        safe_release(m_transform);
    }
    if (m_mf_started) {
        MFShutdown();
        m_mf_started = false;
    }
    m_initialized = false;
}

} // namespace penstream::encode
