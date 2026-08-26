#include "Core/PreRequisites.h"
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
#ifdef TE_SUPPORT_METAL
#include "Renderer/Metal/MetalShader.hpp"
#endif

TERef<Shader> Shader::Create(const TEString &vertexSrc, const TEString &fragmentSrc)
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return CreateRef<OpenGLShader>(vertexSrc, fragmentSrc);
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return CreateRef<OpenGLESShader>(vertexSrc, fragmentSrc);
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return CreateRef<VulkanShader>(vertexSrc, fragmentSrc);
#endif
#ifdef TE_SUPPORT_DIRECTX11
    case GraphicsAPI::DirectX11:
        return CreateRef<DirectX11Shader>(vertexSrc, fragmentSrc);
#endif
#ifdef TE_SUPPORT_METAL
    case GraphicsAPI::Metal:
        return CreateRef<MetalShader>(vertexSrc, fragmentSrc);
#endif
    default:
        return nullptr;
    }
}
