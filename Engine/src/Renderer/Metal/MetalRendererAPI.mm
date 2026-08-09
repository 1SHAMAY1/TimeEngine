#include "Renderer/Metal/MetalRendererAPI.hpp"
#include "Renderer/RendererContext.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#endif

namespace TE
{

void MetalRendererAPI::Init()
{
    TE_CORE_INFO("MetalRendererAPI initialized.");
}

void MetalRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
}

void MetalRendererAPI::SetClearColor(const glm::vec4 &color)
{
}

void MetalRendererAPI::Clear()
{
}

void MetalRendererAPI::DrawIndexed(uint32_t vao, uint32_t indexCount)
{
}

void MetalRendererAPI::SetBlendMode(int blendMode)
{
}

bool MetalRendererAPI::LoadLoader(void *(*loadProc)(const char *))
{
    return true;
}

std::string MetalRendererAPI::GetVersionString()
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device)
    {
        return std::string("Metal ") + [device name].UTF8String;
    }
#endif
    return "Metal 1.0";
}

std::string MetalRendererAPI::GetGPUVendor()
{
    return "Apple";
}

std::string MetalRendererAPI::GetGPURenderer()
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device)
    {
        return [device name].UTF8String;
    }
#endif
    return "Apple Metal GPU";
}

void MetalRendererAPI::GetViewport(int *viewport)
{
}

void MetalRendererAPI::GetClearColor(float *color)
{
}

void MetalRendererAPI::ReadPixelsRGBA(int x, int y, int width, int height, void *outPixels)
{
}

void MetalRendererAPI::SetBlendFunc(BlendFactor src, BlendFactor dst)
{
}

void MetalRendererAPI::SetBlendFuncSeparate(BlendFactor srcRGB, BlendFactor dstRGB, BlendFactor srcAlpha,
                                             BlendFactor dstAlpha)
{
}

} // namespace TE
