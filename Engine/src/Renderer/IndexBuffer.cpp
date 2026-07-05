#include "Renderer/IndexBuffer.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLIndexBuffer.hpp"
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESIndexBuffer.hpp"
#endif
#include "Renderer/DirectX11/DirectX11IndexBuffer.hpp"
#include "Renderer/Vulkan/VulkanIndexBuffer.hpp"

namespace TE
{
IndexBuffer *IndexBuffer::Create(uint32_t *indices, uint32_t Count)
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::OpenGL:
        return new OpenGLIndexBuffer(indices, Count);
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return new OpenGLESIndexBuffer(indices, Count);
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
    case GraphicsAPI::Vulkan:
        return new VulkanIndexBuffer(indices, Count);
    case GraphicsAPI::DirectX11:
        return new DirectX11IndexBuffer(indices, Count);
    default:
        return nullptr;
    }
}
} // namespace TE
