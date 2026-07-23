#include "Renderer/RendererAPI.hpp"
#include "Renderer/RendererContext.hpp"

#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLRendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_OPENGLES
#include "Renderer/OpenGLES/OpenGLESRendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanRendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#endif

namespace TE
{
GraphicsAPI RendererAPI::GetAPI() { return RendererContext::GetAPI(); }

std::unique_ptr<RendererAPI> RendererAPI::Create()
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::None:
        return nullptr;
    case GraphicsAPI::OpenGL:
#ifdef TE_SUPPORT_OPENGL
        return std::make_unique<OpenGLRendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::OpenGLES:
#ifdef TE_SUPPORT_OPENGLES
        return std::make_unique<OpenGLESRendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::Vulkan:
#ifdef TE_SUPPORT_VULKAN
        return std::make_unique<VulkanRendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::DirectX11:
#ifdef TE_SUPPORT_DIRECTX11
        return std::make_unique<DirectX11RendererAPI>();
#else
        return nullptr;
#endif
    }
    return nullptr;
}
} // namespace TE
