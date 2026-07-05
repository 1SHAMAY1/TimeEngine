#include "Renderer/OpenGLES/OpenGLESRendererAPI.hpp"
#include "Renderer/RendererContext.hpp"
#include <glad/glad.h>
#include <iostream>

// NOTE: RendererAPI::GetAPI() and RendererAPI::Create() are defined in
// DirectX11RendererAPI.cpp so that all backends are visible in one place.

namespace TE
{

// ---------------------------------------------------------------------------
// OpenGLESRendererAPI implementation
// ---------------------------------------------------------------------------

static GLenum GetGLESBlendFactor(BlendFactor factor)
{
    switch (factor)
    {
    case BlendFactor::Zero:
        return GL_ZERO;
    case BlendFactor::One:
        return GL_ONE;
    case BlendFactor::SrcAlpha:
        return GL_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha:
        return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DstColor:
        return GL_DST_COLOR;
    }
    return GL_ONE;
}

void OpenGLESRendererAPI::Init()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    std::cout << "[OpenGL ES] Initialized OpenGLESRendererAPI\n";
}

void OpenGLESRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void OpenGLESRendererAPI::SetClearColor(const glm::vec4 &color) { glClearColor(color.r, color.g, color.b, color.a); }

void OpenGLESRendererAPI::Clear() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void OpenGLESRendererAPI::DrawIndexed(uint32_t vao, uint32_t indexCount)
{
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
}

void OpenGLESRendererAPI::SetBlendMode(int blendMode)
{
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
    else // Normal alpha
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }
}

bool OpenGLESRendererAPI::LoadLoader(void *(*loadProc)(const char *))
{
    return gladLoadGLLoader((GLADloadproc)loadProc);
}

std::string OpenGLESRendererAPI::GetVersionString()
{
    const char *version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    return version ? version : "Unknown";
}

std::string OpenGLESRendererAPI::GetGPUVendor()
{
    const char *vendor = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
    return vendor ? vendor : "Unknown";
}

std::string OpenGLESRendererAPI::GetGPURenderer()
{
    const char *renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    return renderer ? renderer : "Unknown";
}

void OpenGLESRendererAPI::GetViewport(int *viewport) { glGetIntegerv(GL_VIEWPORT, viewport); }

void OpenGLESRendererAPI::GetClearColor(float *color) { glGetFloatv(GL_COLOR_CLEAR_VALUE, color); }

void OpenGLESRendererAPI::ReadPixelsRGBA(int x, int y, int width, int height, void *outPixels)
{
    glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, outPixels);
}

void OpenGLESRendererAPI::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
    glBlendFunc(GetGLESBlendFactor(src), GetGLESBlendFactor(dst));
}

void OpenGLESRendererAPI::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                               BlendFactor dstAlpha)
{
    glBlendFuncSeparate(GetGLESBlendFactor(srcRGB), GetGLESBlendFactor(dstRGB), GetGLESBlendFactor(srcAlpha),
                        GetGLESBlendFactor(dstAlpha));
}

} // namespace TE
