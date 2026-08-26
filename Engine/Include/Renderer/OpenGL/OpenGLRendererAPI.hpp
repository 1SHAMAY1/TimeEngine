#pragma once
#include "Renderer/RendererAPI.hpp"

class OpenGLRendererAPI : public RendererAPI
{
public:
    virtual ~OpenGLRendererAPI() override = default;

    virtual void Init() override;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    virtual void SetClearColor(const glm::vec4 &color) override;
    virtual void Clear() override;
    virtual void DrawIndexed(uint32_t vao, uint32_t indexCount) override;
    virtual void SetBlendMode(int blendMode) override;

    virtual bool LoadLoader(void *(*loadProc)(const char *)) override;
    virtual TEString GetVersionString() override;
    virtual TEString GetGPUVendor() override;
    virtual TEString GetGPURenderer() override;

    virtual void GetViewport(int *viewport) override;
    virtual void GetClearColor(float *color) override;
    virtual void ReadPixelsRGBA(int x, int y, int width, int height, void *outPixels) override;
    virtual void SetBlendFunc(BlendFactor src, BlendFactor dst) override;
    virtual void SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                      BlendFactor dstAlpha) override;
};
