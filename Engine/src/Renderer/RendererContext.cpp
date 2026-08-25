#include "Core/PreRequisites.h"
#include "Renderer/RendererContext.hpp"
#include "Renderer/RenderCommand.hpp"

#ifdef _WIN32
// For GPU selection exports
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

GraphicsAPI RendererContext::s_API = GraphicsAPI::OpenGL;

void RendererContext::SetAPI(GraphicsAPI api) { s_API = api; }

GraphicsAPI RendererContext::GetAPI() { return s_API; }

void RendererContext::EnableBestGPU()
{
    // This function is a no-op except for the exports above (which are always present if this TU is linked)
    // Call before context creation for best effect
}

TEString RendererContext::GetGPUVendor() { return RenderCommand::GetGPUVendor(); }

TEString RendererContext::GetGPURenderer() { return RenderCommand::GetGPURenderer(); }

TEString RendererContext::GetGPUType()
{
    TEString vendor = GetGPUVendor();
    TEString renderer = GetGPURenderer();
    // Heuristic: NVIDIA/AMD = Dedicated, Intel = Integrated
    if (vendor.find("NVIDIA") != TEString::npos || renderer.find("NVIDIA") != TEString::npos)
        return "Dedicated";
    if (vendor.find("AMD") != TEString::npos || renderer.find("AMD") != TEString::npos)
        return "Dedicated";
    if (vendor.find("Intel") != TEString::npos || renderer.find("Intel") != TEString::npos)
        return "Integrated";
    return "Unknown";
}

