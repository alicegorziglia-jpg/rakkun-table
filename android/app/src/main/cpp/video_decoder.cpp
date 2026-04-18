#include "video_decoder.h"
#include <android/log.h>

#define LOG_TAG "PenStreamDecoder"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace penstream::android {

VideoDecoder::VideoDecoder()
    : m_codec(nullptr)
    , m_format(nullptr)
    , m_width(1920)
    , m_height(1080)
    , m_initialized(false)
{}

VideoDecoder::~VideoDecoder() {
    release();
}

bool VideoDecoder::initialize(int32_t width, int32_t height) {
    m_width = width;
    m_height = height;

    // Create media format for H.264
    m_format = AMediaFormat_new();
    AMediaFormat_setString(m_format, AMEDIAFORMAT_KEY_MIME, AMEDIAFORMAT_MIME_TYPE_VIDEO_AVC);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_WIDTH, width);
    AMediaFormat_setInt32(m_format, AMEDIAFORMAT_KEY_HEIGHT, height);

    // Create decoder
    m_codec = AMediaCodec_createDecoderByType(AMEDIAFORMAT_MIME_TYPE_VIDEO_AVC);
    if (m_codec == nullptr) {
        LOGE("Failed to create decoder");
        return false;
    }

    // Configure decoder
    media_status_t status = AMediaCodec_configure(
        m_codec,
        m_format,
        nullptr, // surface - will be set later
        nullptr, // crypto
        0        // flags
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
    LOGI("Decoder initialized: %dx%d", width, height);
    return true;
}

bool VideoDecoder::decode(const std::vector<uint8_t>& encoded_data) {
    if (!m_initialized || encoded_data.empty()) {
        return false;
    }

    // Get input buffer
    ssize_t index = AMediaCodec_dequeueInputBuffer(m_codec, 10000);
    if (index < 0) {
        return false;
    }

    // Get input buffer info
    size_t size;
    uint8_t* buffer = AMediaCodec_getInputBuffer(m_codec, index, &size);
    if (buffer == nullptr) {
        return false;
    }

    // Copy encoded data to input buffer
    size_t copy_size = std::min(size, encoded_data.size());
    memcpy(buffer, encoded_data.data(), copy_size);

    // Queue input buffer
    AMediaCodec_queueInputBuffer(
        m_codec,
        index,
        0,
        copy_size,
        0, // presentation time (could extract from packet)
        0  // flags
    );

    // Get output buffer
    AMediaCodecBufferInfo info;
    index = AMediaCodec_dequeueOutputBuffer(m_codec, &info, 0);
    if (index >= 0) {
        // Release output buffer (renderer will handle display)
        AMediaCodec_releaseOutputBuffer(m_codec, index, true);
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
}

} // namespace penstream::android
