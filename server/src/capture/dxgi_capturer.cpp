#include "dxgi_capturer.h"
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace penstream::capture {

DXGICapturer::DXGICapturer()
    : m_device(nullptr)
    , m_context(nullptr)
    , m_duplication(nullptr)
    , m_staging_texture(nullptr)
    , m_width(0)
    , m_height(0)
    , m_initialized(false)
{}

DXGICapturer::~DXGICapturer() {
    shutdown();
}

bool DXGICapturer::initialize() {
    // On hybrid-GPU systems (NVIDIA Optimus, AMD hybrid) D3D11CreateDevice(nullptr,...)
    // may create the device on the iGPU while the desktop output belongs to the dGPU.
    // DuplicateOutput requires that the device and the output share the same adapter.
    // We therefore enumerate every adapter/output pair and try each one until we find
    // a combination that succeeds with DuplicateOutput.

    IDXGIFactory1* factory = nullptr;
    HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory));
    if (FAILED(hr)) {
        spdlog::error("CreateDXGIFactory1 failed: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    bool found = false;
    for (UINT adapterIdx = 0; !found; ++adapterIdx) {
        IDXGIAdapter1* adapter = nullptr;
        if (factory->EnumAdapters1(adapterIdx, &adapter) == DXGI_ERROR_NOT_FOUND) {
            break; // No more adapters
        }

        DXGI_ADAPTER_DESC1 adapterDesc;
        adapter->GetDesc1(&adapterDesc);

        // Skip Microsoft Basic Render Driver (software fallback)
        if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            adapter->Release();
            continue;
        }

        // Try to create D3D11 device on this specific adapter
        ID3D11Device*        device  = nullptr;
        ID3D11DeviceContext* context = nullptr;
        hr = D3D11CreateDevice(
            adapter,
            D3D_DRIVER_TYPE_UNKNOWN, // must be UNKNOWN when an explicit adapter is given
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            feature_levels,
            ARRAYSIZE(feature_levels),
            D3D11_SDK_VERSION,
            &device,
            nullptr,
            &context
        );

        if (FAILED(hr)) {
            spdlog::warn("D3D11CreateDevice failed on adapter {}: 0x{:08X}", adapterIdx,
                         static_cast<uint32_t>(hr));
            adapter->Release();
            continue;
        }

        // Enumerate outputs on this adapter
        for (UINT outputIdx = 0; ; ++outputIdx) {
            IDXGIOutput* output = nullptr;
            if (adapter->EnumOutputs(outputIdx, &output) == DXGI_ERROR_NOT_FOUND) {
                break;
            }

            IDXGIOutput1* output1 = nullptr;
            if (FAILED(output->QueryInterface(__uuidof(IDXGIOutput1),
                                              reinterpret_cast<void**>(&output1)))) {
                output->Release();
                continue;
            }

            DXGI_OUTPUT_DESC desc;
            output1->GetDesc(&desc);

            // Skip outputs not attached to the desktop (e.g. disconnected monitors)
            if (!desc.AttachedToDesktop) {
                output1->Release();
                output->Release();
                continue;
            }

            IDXGIOutputDuplication* duplication = nullptr;
            hr = output1->DuplicateOutput(device, &duplication);
            output1->Release();
            output->Release();

            if (SUCCEEDED(hr)) {
                spdlog::info("Using adapter {} output {}", adapterIdx, outputIdx);
                m_device      = device;
                m_context     = context;
                m_duplication = duplication;
                m_width  = static_cast<uint32_t>(desc.DesktopCoordinates.right  - desc.DesktopCoordinates.left);
                m_height = static_cast<uint32_t>(desc.DesktopCoordinates.bottom - desc.DesktopCoordinates.top);
                found = true;
                break;
            }

            spdlog::warn("DuplicateOutput failed on adapter {} output {}: 0x{:08X}",
                         adapterIdx, outputIdx, static_cast<uint32_t>(hr));

            if (hr == static_cast<HRESULT>(0x887A0004)) { // DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
                spdlog::error("DXGI_ERROR_NOT_CURRENTLY_AVAILABLE — possible causes:");
                spdlog::error("  - Running over Remote Desktop (RDP)");
                spdlog::error("  - Running inside a VM without GPU passthrough");
                spdlog::error("  - Another process holds an exclusive fullscreen surface");
            }
        }

        if (!found) {
            device->Release();
            context->Release();
        }
        adapter->Release();
    }

    factory->Release();

    if (!found) {
        spdlog::error("No adapter/output combination succeeded with DuplicateOutput.");
        return false;
    }

    // Create staging texture for CPU readback
    if (!create_staging_texture()) {
        return false;
    }

    m_initialized = true;
    spdlog::info("DXGI capturer initialized: {}x{}", m_width, m_height);
    return true;
}

// Kept for API compatibility — logic is now inlined in initialize().
bool DXGICapturer::create_device() { return true; }

bool DXGICapturer::create_duplication() { return true; }

bool DXGICapturer::create_staging_texture() {

    D3D11_TEXTURE2D_DESC staging_desc = {};
    staging_desc.Width = m_width;
    staging_desc.Height = m_height;
    staging_desc.MipLevels = 1;
    staging_desc.ArraySize = 1;
    staging_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    staging_desc.SampleDesc.Count = 1;
    staging_desc.Usage = D3D11_USAGE_STAGING;
    staging_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    HRESULT hr = m_device->CreateTexture2D(&staging_desc, nullptr, &m_staging_texture);
    if (FAILED(hr)) {
        spdlog::error("Failed to create staging texture: 0x{:08X}", static_cast<uint32_t>(hr));
        return false;
    }

    return true;
}

bool DXGICapturer::capture_frame(Frame& out_frame) {
    if (!m_initialized) {
        return false;
    }

    IDXGIResource* resource = nullptr;
    DXGI_OUTDUPL_FRAME_INFO frame_info;

    HRESULT hr = m_duplication->AcquireNextFrame(10, &frame_info, &resource);

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return false; // No frame available
    }

    if (FAILED(hr)) {
        spdlog::warn("AcquireNextFrame failed: 0x{:08X}", hr);
        return false;
    }

    // Copiar a staging texture
    ID3D11Texture2D* texture = nullptr;
    hr = resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture));
    resource->Release();

    if (FAILED(hr)) {
        return false;
    }

    m_context->CopyResource(m_staging_texture, texture);
    texture->Release();

    // Mapear staging texture
    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = m_context->Map(m_staging_texture, 0, D3D11_MAP_READ, 0, &mapped);

    if (FAILED(hr)) {
        return false;
    }

    // Copiar datos
    out_frame.data.resize(m_width * m_height * 4);
    memcpy(out_frame.data.data(), mapped.pData, out_frame.data.size());

    m_context->Unmap(m_staging_texture, 0);

    out_frame.width = m_width;
    out_frame.height = m_height;
    out_frame.timestamp_us = frame_info.LastPresentTime.QuadPart;
    out_frame.is_keyframe = false; // Los keyframes los decide el encoder

    // Release frame
    m_duplication->ReleaseFrame();

    return true;
}

void DXGICapturer::shutdown() {
    if (m_duplication) {
        m_duplication->ReleaseFrame();
        m_duplication->Release();
        m_duplication = nullptr;
    }

    if (m_staging_texture) {
        m_staging_texture->Release();
        m_staging_texture = nullptr;
    }

    if (m_context) {
        m_context->Release();
        m_context = nullptr;
    }

    if (m_device) {
        m_device->Release();
        m_device = nullptr;
    }

    m_initialized = false;
}

} // namespace penstream::capture
