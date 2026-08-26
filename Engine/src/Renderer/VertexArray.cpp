#include "Core/PreRequisites.h"
#include "Renderer/VertexArray.hpp"
#include "Renderer/GraphicsAPI.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLVertexArray.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESVertexArray.hpp"
#endif
#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11VertexArray.hpp"
#endif
#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanVertexArray.hpp"
#endif
#ifdef TE_SUPPORT_METAL
#include "Renderer/Metal/MetalVertexArray.hpp"
#endif

TERef<VertexArray> VertexArray::Create()
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return CreateRef<OpenGLVertexArray>();
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return CreateRef<OpenGLESVertexArray>();
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return CreateRef<VulkanVertexArray>();
#endif
#ifdef TE_SUPPORT_DIRECTX11
    case GraphicsAPI::DirectX11:
        return CreateRef<DirectX11VertexArray>();
#endif
#ifdef TE_SUPPORT_METAL
    case GraphicsAPI::Metal:
        return CreateRef<MetalVertexArray>();
#endif
    default:
        return nullptr;
    }
}
