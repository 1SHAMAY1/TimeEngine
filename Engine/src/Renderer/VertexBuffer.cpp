#include "Renderer/VertexBuffer.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/OpenGL/OpenGLVertexBuffer.hpp"

namespace TE {
    VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size) {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::None: return nullptr;
        case GraphicsAPI::OpenGL: return new OpenGLVertexBuffer(vertices, size);
        case GraphicsAPI::OpenGLES: /* TODO */ break;
        case GraphicsAPI::Vulkan: /* TODO */ break;
        }
        return nullptr;
    }
}
