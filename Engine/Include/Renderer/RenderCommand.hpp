#pragma once
#include "Core/PreRequisites.h"
#include "Renderer/RendererAPI.hpp"
#include <glm/glm.hpp>

class TE_API RenderCommand
{
public:
    static void Init() { s_RendererAPI->Init(); }
    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        s_RendererAPI->SetViewport(x, y, width, height);
    }
    static void SetClearColor(const glm::vec4 &color) { s_RendererAPI->SetClearColor(color); }
    static void Clear() { s_RendererAPI->Clear(); }
    static void DrawIndexed(uint32_t vao, uint32_t indexCount) { s_RendererAPI->DrawIndexed(vao, indexCount); }
    static void SetBlendMode(int blendMode) { s_RendererAPI->SetBlendMode(blendMode); }

    static bool LoadLoader(void *(*loadProc)(const char *))
    {
        return s_RendererAPI ? s_RendererAPI->LoadLoader(loadProc) : false;
    }
    static TEString GetVersionString()
    {
        return s_RendererAPI ? s_RendererAPI->GetVersionString() : "Unknown OpenGL Version";
    }
    static TEString GetGPUVendor() { return s_RendererAPI ? s_RendererAPI->GetGPUVendor() : "Unknown Vendor"; }
    static TEString GetGPURenderer() { return s_RendererAPI ? s_RendererAPI->GetGPURenderer() : "Unknown Renderer"; }

    static void GetViewport(int *viewport) { s_RendererAPI->GetViewport(viewport); }
    static void GetClearColor(float *color) { s_RendererAPI->GetClearColor(color); }
    static void ReadPixelsRGBA(int x, int y, int width, int height, void *outPixels)
    {
        s_RendererAPI->ReadPixelsRGBA(x, y, width, height, outPixels);
    }
    static void SetBlendFunc(BlendFactor src, BlendFactor dst) { s_RendererAPI->SetBlendFunc(src, dst); }
    static void SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha, BlendFactor dstAlpha)
    {
        s_RendererAPI->SetBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }

    static RendererAPI *GetAPIInstance() { return s_RendererAPI.get(); }

private:
    static TEScope<RendererAPI> s_RendererAPI;
};
