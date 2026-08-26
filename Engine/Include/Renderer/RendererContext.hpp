#pragma once
#include "Core/PreRequisites.h"
#include "Renderer/GraphicsAPI.hpp"

class TE_API RendererContext
{
public:
    static void SetAPI(GraphicsAPI api);
    static GraphicsAPI GetAPI();

    // GPU selection and detection
    static void EnableBestGPU(); // Call before context creation
    static TEString GetGPUVendor();
    static TEString GetGPURenderer();
    static TEString GetGPUType(); // "Dedicated", "Integrated", or "Unknown"
private:
    static GraphicsAPI s_API;
};
