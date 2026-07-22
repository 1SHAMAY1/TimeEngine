#include "Renderer/Shader.hpp"
#include "Renderer/GraphicsAPI.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLShader.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESShader.hpp"
#endif
#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11Shader.hpp"
#endif
#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanShader.hpp"
#endif

namespace TE
{
Shader *Shader::Create(const std::string &vertexSrc, const std::string &fragmentSrc)
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return new OpenGLShader(vertexSrc, fragmentSrc);
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return new OpenGLESShader(vertexSrc, fragmentSrc);
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return new VulkanShader(vertexSrc, fragmentSrc);
#endif
#ifdef TE_SUPPORT_DIRECTX11
    case GraphicsAPI::DirectX11:
        return new DirectX11Shader(vertexSrc, fragmentSrc);
#endif
    default:
        return nullptr;
    }
}
} // namespace TE
