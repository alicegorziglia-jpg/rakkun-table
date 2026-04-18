#include "renderer.h"
#include <android/log.h>

#define LOG_TAG "PenStreamRenderer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace penstream::android {

Renderer::Renderer()
    : m_window(nullptr)
    , m_display(EGL_NO_DISPLAY)
    , m_surface(EGL_NO_SURFACE)
    , m_context(EGL_NO_CONTEXT)
    , m_program(0)
    , m_texture(0)
    , m_initialized(false)
{}

Renderer::~Renderer() {
    release();
}

bool Renderer::initialize() {
    if (m_display == EGL_NO_DISPLAY) {
        m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (m_display == EGL_NO_DISPLAY) {
            LOGE("Failed to get EGL display");
            return false;
        }

        if (!eglInitialize(m_display, nullptr, nullptr)) {
            LOGE("Failed to initialize EGL");
            return false;
        }
    }

    m_initialized = true;
    LOGI("Renderer initialized");
    return true;
}

void Renderer::set_surface(ANativeWindow* window) {
    if (m_window == window) {
        return;
    }

    // Cleanup old surface
    if (m_surface != EGL_NO_SURFACE) {
        eglDestroySurface(m_display, m_surface);
        m_surface = EGL_NO_SURFACE;
    }

    m_window = window;

    if (m_window != nullptr) {
        // Create new surface
        const EGLint attribs[] = {
            EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
            EGL_NONE
        };
        m_surface = eglCreateWindowSurface(m_display, EGL_DEFAULT_CONFIG, m_window, attribs);

        if (m_surface == EGL_NO_SURFACE) {
            LOGE("Failed to create EGL surface");
            return;
        }

        // Create context if needed
        if (m_context == EGL_NO_CONTEXT) {
            const EGLint context_attribs[] = {
                EGL_CONTEXT_CLIENT_VERSION, 3,
                EGL_NONE
            };
            m_context = eglCreateContext(m_display, EGL_DEFAULT_CONFIG, EGL_NO_CONTEXT, context_attribs);
        }

        eglMakeCurrent(m_display, m_surface, m_surface, m_context);

        // Setup OpenGL
        setup_shaders();
    }
}

void Renderer::setup_shaders() {
    // Simple passthrough shader for YUV->RGB conversion
    const char* vertex_shader = R"(
        #version 300 es
        layout(location = 0) in vec4 a_position;
        layout(location = 1) in vec2 a_texCoord;
        out vec2 v_texCoord;
        void main() {
            gl_Position = a_position;
            v_texCoord = a_texCoord;
        }
    )";

    const char* fragment_shader = R"(
        #version 300 es
        precision mediump float;
        in vec2 v_texCoord;
        uniform sampler2D u_texture;
        out vec4 fragColor;
        void main() {
            fragColor = texture(u_texture, v_texCoord);
        }
    )";

    // Compile shaders (simplified - would need full compilation code)
    // For now, just clear to a color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void Renderer::render_frame(const void* y_data, const void* uv_data,
                            int32_t width, int32_t height) {
    if (!m_initialized || m_surface == EGL_NO_SURFACE) {
        return;
    }

    // Make context current
    eglMakeCurrent(m_display, m_surface, m_surface, m_context);

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    // Render frame (would bind texture and draw quad)
    // Simplified for scaffolding

    // Swap buffers
    eglSwapBuffers(m_display, m_surface);
}

void Renderer::release() {
    if (m_context != EGL_NO_CONTEXT) {
        eglDestroyContext(m_display, m_context);
        m_context = EGL_NO_CONTEXT;
    }

    if (m_surface != EGL_NO_SURFACE) {
        eglDestroySurface(m_display, m_surface);
        m_surface = EGL_NO_SURFACE;
    }

    if (m_display != EGL_NO_DISPLAY) {
        eglTerminate(m_display);
        m_display = EGL_NO_DISPLAY;
    }

    m_window = nullptr;
    m_initialized = false;
}

} // namespace penstream::android
