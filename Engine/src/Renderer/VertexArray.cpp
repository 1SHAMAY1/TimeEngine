#include "Renderer/VertexArray.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLVertexArray.hpp"
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESVertexArray.hpp"
#endif
#include "Renderer/DirectX11/DirectX11VertexArray.hpp"
#include "Renderer/Vulkan/VulkanVertexArray.hpp"

namespace TE
{
VertexArray *VertexArray::Create()
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::OpenGL:
        return new OpenGLVertexArray();
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return new OpenGLESVertexArray();
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
    case GraphicsAPI::Vulkan:
        return new VulkanVertexArray();
    case GraphicsAPI::DirectX11:
        return new DirectX11VertexArray();
    default:
        return nullptr;
    }
}
} // namespace TE
