#include "Renderer/RendererContext.hpp"
#include <glad/glad.h>
#include <string>

#ifdef _WIN32
// For GPU selection exports
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

namespace TE {
	GraphicsAPI RendererContext::s_API = GraphicsAPI::OpenGL;

	void RendererContext::SetAPI(GraphicsAPI api) {
		s_API = api;
	}

	GraphicsAPI RendererContext::GetAPI() {
		return s_API;
	}

    void RendererContext::EnableBestGPU() {
        // This function is a no-op except for the exports above (which are always present if this TU is linked)
        // Call before context creation for best effect
    }

    std::string RendererContext::GetGPUVendor() {
        const char* vendor = (const char*)glGetString(GL_VENDOR);
        return vendor ? vendor : "Unknown";
    }

    std::string RendererContext::GetGPURenderer() {
        const char* renderer = (const char*)glGetString(GL_RENDERER);
        return renderer ? renderer : "Unknown";
    }

    std::string RendererContext::GetGPUType() {
        std::string vendor = GetGPUVendor();
        std::string renderer = GetGPURenderer();
        // Heuristic: NVIDIA/AMD = Dedicated, Intel = Integrated
        if (vendor.find("NVIDIA") != std::string::npos || renderer.find("NVIDIA") != std::string::npos)
            return "Dedicated";
        if (vendor.find("AMD") != std::string::npos || renderer.find("AMD") != std::string::npos)
            return "Dedicated";
        if (vendor.find("Intel") != std::string::npos || renderer.find("Intel") != std::string::npos)
            return "Integrated";
        return "Unknown";
    }
}