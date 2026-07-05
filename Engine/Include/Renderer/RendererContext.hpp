#pragma once
#include "Core/PreRequisites.h"
#include "Renderer/GraphicsAPI.hpp"
#include <string>

namespace TE {
    class TE_API RendererContext {
    public:
        static void SetAPI(GraphicsAPI api);
        static GraphicsAPI GetAPI();

        // GPU selection and detection
        static void EnableBestGPU(); // Call before context creation
        static std::string GetGPUVendor();
        static std::string GetGPURenderer();
        static std::string GetGPUType(); // "Dedicated", "Integrated", or "Unknown"
    private:
        static GraphicsAPI s_API;
    };
}
