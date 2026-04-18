#pragma once

#include <vector>
#include <cstdint>
#include <android/media_mediaformat.h>
#include <android/codec.h>

namespace penstream::android {

class VideoDecoder {
public:
    VideoDecoder();
    ~VideoDecoder();

    bool initialize(int32_t width, int32_t height);
    bool decode(const std::vector<uint8_t>& encoded_data);
    void release();

    int32_t get_output_width() const { return m_width; }
    int32_t get_output_height() const { return m_height; }

private:
    AMediaCodec* m_codec;
    AMediaFormat* m_format;
    int32_t m_width;
    int32_t m_height;
    bool m_initialized;
};

} // namespace penstream::android
