#include "Renderer/OpenGL/OpenGLRendererAPI.hpp"
#include "Renderer/RendererContext.hpp"
#include <glad/glad.h>

// NOTE: RendererAPI::GetAPI() and RendererAPI::Create() are defined in
// DirectX11RendererAPI.cpp so that all backends are visible in one place.

namespace TE {


    void OpenGLRendererAPI::Init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        glViewport(x, y, width, height);
    }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::DrawIndexed(uint32_t vao, uint32_t indexCount) {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRendererAPI::SetBlendMode(int blendMode) {
        glEnable(GL_BLEND);
        if (blendMode == 1) // Additive
        {
            glBlendFunc(GL_ONE, GL_ONE);
            glDisable(GL_DEPTH_TEST);
        }
        else if (blendMode == 2) // Multiplicative
        {
            glBlendFunc(GL_DST_COLOR, GL_ZERO);
            glDisable(GL_DEPTH_TEST);
        }
        else // Normal
        {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);
        }
    }

    bool OpenGLRendererAPI::LoadLoader(void* (*loadProc)(const char*)) {
        return gladLoadGLLoader((GLADloadproc)loadProc);
    }

    std::string OpenGLRendererAPI::GetVersionString() {
        const char* version = (const char*)glGetString(GL_VERSION);
        return version ? version : "Unknown";
    }

    std::string OpenGLRendererAPI::GetGPUVendor() {
        const char* vendor = (const char*)glGetString(GL_VENDOR);
        return vendor ? vendor : "Unknown";
    }

    std::string OpenGLRendererAPI::GetGPURenderer() {
        const char* renderer = (const char*)glGetString(GL_RENDERER);
        return renderer ? renderer : "Unknown";
    }

    static GLenum GetGLBlendFactor(BlendFactor factor) {
        switch (factor) {
            case BlendFactor::Zero: return GL_ZERO;
            case BlendFactor::One: return GL_ONE;
            case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
            case BlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
            case BlendFactor::DstColor: return GL_DST_COLOR;
        }
        return GL_ONE;
    }

    void OpenGLRendererAPI::GetViewport(int* viewport) {
        glGetIntegerv(GL_VIEWPORT, viewport);
    }

    void OpenGLRendererAPI::GetClearColor(float* color) {
        glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    }

    void OpenGLRendererAPI::ReadPixelsRGBA(int x, int y, int width, int height, void* outPixels) {
        glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, outPixels);
    }

    void OpenGLRendererAPI::SetBlendFunc(BlendFactor src, BlendFactor dst) {
        glBlendFunc(GetGLBlendFactor(src), GetGLBlendFactor(dst));
    }

    void OpenGLRendererAPI::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha) {
        glBlendFuncSeparate(GetGLBlendFactor(srcRGB), GetGLBlendFactor(dstRGB), GetGLBlendFactor(srcAlpha), GetGLBlendFactor(dstAlpha));
    }

}
