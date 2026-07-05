#include "Renderer/Shader.hpp"
#include "Renderer/GraphicsAPI.hpp"
#include "Renderer/OpenGL/OpenGLShader.hpp"
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESShader.hpp"
#endif
#include "Renderer/DirectX11/DirectX11Shader.hpp"
#include "Renderer/Vulkan/VulkanShader.hpp"

namespace TE
{
Shader *Shader::Create(const std::string &vertexSrc, const std::string &fragmentSrc)
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::OpenGL:
        return new OpenGLShader(vertexSrc, fragmentSrc);
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return new OpenGLESShader(vertexSrc, fragmentSrc);
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
    case GraphicsAPI::Vulkan:
        return new VulkanShader(vertexSrc, fragmentSrc);
    case GraphicsAPI::DirectX11:
        return new DirectX11Shader(vertexSrc, fragmentSrc);
    default:
        return nullptr;
    }
}
} // namespace TE
