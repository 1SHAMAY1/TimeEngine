#include "Renderer/IndexBuffer.hpp"
#include "Core/PreRequisites.h"
#include "Renderer/GraphicsAPI.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLIndexBuffer.hpp"
#endif
#include "Renderer/RendererContext.hpp"
#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESIndexBuffer.hpp"
#endif
#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11IndexBuffer.hpp"
#endif
#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanIndexBuffer.hpp"
#endif
#ifdef TE_SUPPORT_METAL
#include "Renderer/Metal/MetalIndexBuffer.hpp"
#endif

TERef<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t Count)
{
    switch (RendererContext::GetAPI())
    {
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return CreateRef<OpenGLIndexBuffer>(indices, Count);
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return CreateRef<OpenGLESIndexBuffer>(indices, Count);
#else
    case GraphicsAPI::OpenGLES:
        return nullptr;
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return CreateRef<VulkanIndexBuffer>(indices, Count);
#endif
#ifdef TE_SUPPORT_DIRECTX11
    case GraphicsAPI::DirectX11:
        return CreateRef<DirectX11IndexBuffer>(indices, Count);
#endif
#ifdef TE_SUPPORT_METAL
    case GraphicsAPI::Metal:
        return CreateRef<MetalIndexBuffer>(indices, Count);
#endif
    default:
        return nullptr;
    }
}
