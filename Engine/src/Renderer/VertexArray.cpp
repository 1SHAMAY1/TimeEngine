#include "Renderer/VertexArray.hpp"
#include "Renderer/GraphicsAPI.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLVertexArray.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESVertexArray.hpp"
#endif
#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11VertexArray.hpp"
#endif
#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanVertexArray.hpp"
#endif

namespace TE
{
VertexArray *VertexArray::Create()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return new OpenGLVertexArray();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return new OpenGLESVertexArray();
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return new VulkanVertexArray();
#endif
#ifdef TE_SUPPORT_DIRECTX11
    case GraphicsAPI::DirectX11:
        return new DirectX11VertexArray();
#endif
    default:
        return nullptr;
    }
}
} // namespace TE
