#include "video_decoder.h"
#include <android/log.h>

#define LOG_TAG "PenStreamDecoder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace penstream::android {

VideoDecoder::VideoDecoder()
    : m_codec(nullptr)
    , m_format(nullptr)
    , m_surface(nullptr)
    , m_width(1920)
    , m_height(1080)
    , m_initialized(false)
{}

VideoDecoder::~VideoDecoder() {
    release();
}

void VideoDecoder::set_surface(ANativeWindow* surface) {
    m_surface = surface;

    // If already initialized, reconfigure codec with new surface
    if (m_initialized && m_codec) {
        AMediaCodec_configure(
            m_codec,
            m_format,
            m_surface,
            nullptr,
            0
        );
        LOGI("Decoder reconfigured with new surface");
    }
}

bool VideoDecoder::initialize(int32_t width, int32_t height, ANativeWindow* surface) {
    m_width = width;
    m_height = height;
    m_surface = surface;

    // Create media format for H.264
    m_format = AMediaFormat_new();
    AMediaFormat_setString(m_format, AMEDIAFORMAT_KEY_MIME, AMEDIAFORMAT_MIME_TYPE_VIDEO_AVC);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_HEIGHT, height);

    // Low latency settings
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_LOW_LATENCY, 1);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_OPERATING_RATE, 60);

    // Create decoder
    m_codec = AMediaCodec_createDecoderByType(AMEDIAFORMAT_MIME_TYPE_VIDEO_AVC);
    if (m_codec == nullptr) {
        LOGE("Failed to create decoder");
        return false;
    }

    // Configure decoder with surface for direct rendering
    media_status_t status = AMediaCodec_configure(
        m_codec,
        m_format,
        m_surface,
        nullptr,
        0  // flags - 0 means decoder mode
    );

    if (status != AMEDIA_OK) {
        LOGE("Failed to configure decoder: %d", status);
        return false;
    }

    // Start decoder
    status = AMediaCodec_start(m_codec);
    if (status != AMEDIA_OK) {
        LOGE("Failed to start decoder: %d", status);
        return false;
    }

    m_initialized = true;
    LOGI("Decoder initialized: %dx%d with surface", width, height);
    return true;
}

bool VideoDecoder::decode(const std::vector<uint8_t>& encoded_data, bool is_keyframe) {
    if (!m_initialized || !m_codec || encoded_data.empty()) {
        return false;
    }

    // Get input buffer with timeout
    ssize_t index = AMediaCodec_dequeueInputBuffer(m_codec, 10000);
    if (index < 0) {
        LOGI("No input buffer available (timeout)");
        return false;
    }

    // Get input buffer info
    size_t size;
    uint8_t* buffer = AMediaCodec_getInputBuffer(m_codec, index, &size);
    if (buffer == nullptr) {
        LOGE("Failed to get input buffer");
        return false;
    }

    // Copy encoded data to input buffer
    size_t copy_size = std::min(size, encoded_data.size());
    memcpy(buffer, encoded_data.data(), copy_size);

    // Determine flags
    uint32_t flags = 0;
    if (is_keyframe) {
        flags = AMEDIACODEC_BUFFER_FLAG_KEY_FRAME;
        LOGI("Sending keyframe to decoder");
    }

    // Queue input buffer
    AMediaCodec_queueInputBuffer(
        m_codec,
        index,
        0,
        copy_size,
        0,  // presentation time - could extract from packet
        flags
    );

    // Process output buffers
    AMediaCodecBufferInfo info;
    ssize_t out_index;

    while ((out_index = AMediaCodec_dequeueOutputBuffer(m_codec, &info, 0)) >= 0) {
        if (out_index >= 0) {
            // Release buffer to surface for display
            // render=true means it will be displayed
            AMediaCodec_releaseOutputBuffer(m_codec, out_index, true);
        }
    }

    return true;
}

void VideoDecoder::release() {
    if (m_codec) {
        AMediaCodec_stop(m_codec);
        AMediaCodec_delete(m_codec);
        m_codec = nullptr;
    }

    if (m_format) {
        AMediaFormat_delete(m_format);
        m_format = nullptr;
    }

    m_initialized = false;
    LOGI("Decoder released");
}

} // namespace penstream::android
