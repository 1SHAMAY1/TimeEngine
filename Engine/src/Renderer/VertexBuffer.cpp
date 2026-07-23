#include "Renderer/VertexBuffer.hpp"
#include "Renderer/GraphicsAPI.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLVertexBuffer.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESVertexBuffer.hpp"
#endif
#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11VertexBuffer.hpp"
#endif
#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanVertexBuffer.hpp"
#endif

namespace TE
{
VertexBuffer *VertexBuffer::Create(float *vertices, uint32_t size)
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::None:
        return nullptr;
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return new OpenGLVertexBuffer(vertices, size);
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return new OpenGLESVertexBuffer(vertices, size);
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return new VulkanVertexBuffer(vertices, size);
#endif
#ifdef TE_SUPPORT_DIRECTX11
    case GraphicsAPI::DirectX11:
        return new DirectX11VertexBuffer(vertices, size);
#endif
    }
    return nullptr;
}
} // namespace TE
