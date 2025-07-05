#include "Renderer/Shader.hpp"
#include "Renderer/RendererContext.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLShader.hpp"

namespace TE {
    Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc) {
        switch (RendererContext::GetAPI()) {
        case GraphicsAPI::OpenGL: return new OpenGLShader(vertexSrc, fragmentSrc);
        default: return nullptr;
        }
    }
}
