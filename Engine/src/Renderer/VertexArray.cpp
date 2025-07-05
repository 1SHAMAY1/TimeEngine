#include "Renderer/VertexArray.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLVertexArray.hpp"

namespace TE {
    VertexArray* VertexArray::Create() {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return new OpenGLVertexArray();
        default: return nullptr;
        }
    }
}
