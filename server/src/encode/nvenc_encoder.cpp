#include "nvenc_encoder.h"
#include <spdlog/spdlog.h>

// NVENC includes - require NVIDIA Video Codec SDK
// Download from: https://developer.nvidia.com/nvidia-video-codec-sdk

namespace penstream::encode {

// Function pointers for NVENC API
typedef NVENCSTATUS (NVENCAPI* NvEncodeAPICreateInstance_t)(NV_ENCODE_API_FUNCTION_LIST*);
typedef NVENCSTATUS (NVENCAPI* NvEncodeAPIGetMaxSupportedVersion_t)(uint32_t*);

NVEncoder::NVEncoder()
    : m_nvenc_lib(nullptr)
    , m_nvenc(nullptr)
    , m_encoder(nullptr)
    , m_input_buffer(nullptr)
    , m_registered_resource(nullptr)
    , m_width(0)
    , m_height(0)
    , m_fps(0)
    , m_initialized(false)
{
    m_nvenc = new NV_ENCODE_API_FUNCTION_LIST();
}

NVEncoder::~NVEncoder() {
    shutdown();
    delete m_nvenc;
}

bool NVEncoder::initialize(const EncodeConfig& config) {
    if (!load_nvenc()) {
        spdlog::error("Failed to load NVEnc library");
        return false;
    }

    // Check NVENC version
    uint32_t version = 0;
    if (m_nvenc->nvEncodeAPIGetMaxSupportedVersion(&version) != NV_ENC_SUCCESS) {
        spdlog::error("Failed to get NVENC version");
        return false;
    }
    spdlog::info("NVENC version: {}.{}", (version >> 4) & 0xFF, version & 0xF);

    if (!create_encoder(config)) {
        spdlog::error("Failed to create encoder");
        return false;
    }

    m_width = config.width;
    m_height = config.height;
    m_fps = config.fps;
    m_initialized = true;

    spdlog::info("NVENC encoder initialized: {}x{}@{}fps, {}kbps",
                 config.width, config.height, config.fps, config.bitrate_bps / 1000);
    return true;
}

bool NVEncoder::load_nvenc() {
    m_nvenc_lib = LoadLibraryA("nvEncodeAPI64.dll");
    if (!m_nvenc_lib) {
        spdlog::error("Failed to load nvEncodeAPI64.dll - NVIDIA driver not installed?");
        return false;
    }

    auto create_instance = reinterpret_cast<NvEncodeAPICreateInstance_t>(
        GetProcAddress(m_nvenc_lib, "NvEncodeAPICreateInstance"));

    if (!create_instance) {
        spdlog::error("Failed to get NvEncodeAPICreateInstance");
        return false;
    }

    memset(m_nvenc, 0, sizeof(NV_ENCODE_API_FUNCTION_LIST));
    m_nvenc->version = NV_ENCODE_API_FUNCTION_LIST_VER;

    NVENCSTATUS status = create_instance(m_nvenc);
    if (status != NV_ENC_SUCCESS) {
        spdlog::error("NvEncodeAPICreateInstance failed: {}", status);
        return false;
    }

    // Initialize function pointers
    if (m_nvenc->nvEncOpenEncodeSessionEx == nullptr) {
        spdlog::error("nvEncOpenEncodeSessionEx not available");
        return false;
    }

    return true;
}

bool NVEncoder::create_encoder(const EncodeConfig& config) {
    // For now, create a simple encoder session
    // Full implementation requires D3D11 interop with DXGI capturer

    NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS session_params = {};
    session_params.version = NV_ENC_OPEN_ENCODE_SESSION_EX_PARAMS_VER;
    session_params.device = nullptr; // Will be set from DXGI device
    session_params.deviceType = NV_ENC_DEVICE_TYPE_DIRECTX;
    session_params.apiVersion = NVENCAPI_VERSION;

    // This will be called after DXGI is initialized
    // For scaffolding, we mark as initialized
    spdlog::info("NVENC encoder session will be created with D3D11 device");

    return true;
}

bool NVEncoder::encode(const capture::Frame& frame, std::vector<uint8_t>& out_data) {
    if (!m_initialized) {
        return false;
    }

    // Full implementation:
    // 1. Register D3D11 texture from DXGI capturer
    // 2. Map input buffer
    // 3. Copy frame data
    // 4. Run encode
    // 5. Lock bitstream and copy output

    // Stub for scaffolding - returns empty frame
    out_data.clear();
    return true;
}

void NVEncoder::shutdown() {
    if (m_encoder && m_nvenc && m_nvenc->nvEncDestroyEncoder) {
        m_nvenc->nvEncDestroyEncoder(m_encoder);
        m_encoder = nullptr;
    }

    if (m_nvenc_lib) {
        FreeLibrary(static_cast<HMODULE>(m_nvenc_lib));
        m_nvenc_lib = nullptr;
    }

    m_initialized = false;
}

} // namespace penstream::encode
