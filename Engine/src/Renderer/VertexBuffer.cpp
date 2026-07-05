#include "Renderer/VertexBuffer.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLVertexBuffer.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESVertexBuffer.hpp"
#endif
#include "Renderer/Vulkan/VulkanVertexBuffer.hpp"
#include "Renderer/DirectX11/DirectX11VertexBuffer.hpp"

namespace TE {
    VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::None: return nullptr;
        case GraphicsAPI::OpenGL:    return new OpenGLVertexBuffer(vertices, size);
#if defined(TE_PLATFORM_MOBILE)
        case GraphicsAPI::OpenGLES:  return new OpenGLESVertexBuffer(vertices, size);
#else
        case GraphicsAPI::OpenGLES:  return nullptr;
#endif
        case GraphicsAPI::Vulkan:    return new VulkanVertexBuffer(vertices, size);
        case GraphicsAPI::DirectX11: return new DirectX11VertexBuffer(vertices, size);
        }
        return nullptr;
    }
}
