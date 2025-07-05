#include "Renderer/IndexBuffer.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLIndexBuffer.hpp"

namespace TE {
    IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t Count) {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return new OpenGLIndexBuffer(indices, Count);
        default: return nullptr;
        }
    }
}
