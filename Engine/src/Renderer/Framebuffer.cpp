#include "Renderer/Framebuffer.hpp"
#include "Core/PreRequisites.h"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLFramebuffer.hpp"
#endif
#include "Renderer/RendererContext.hpp"

#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESFramebuffer.hpp"
#endif

#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanFramebuffer.hpp"
#endif

#ifdef TE_SUPPORT_METAL
#include "Renderer/Metal/MetalFramebuffer.hpp"
#endif

TERef<Framebuffer> Framebuffer::Create(const FramebufferSpecification &spec)
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::None:
        return nullptr;
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return CreateRef<OpenGLFramebuffer>(spec);
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return CreateRef<OpenGLESFramebuffer>(spec);
#endif
#ifdef TE_SUPPORT_VULKAN
    case GraphicsAPI::Vulkan:
        return CreateRef<VulkanFramebuffer>(spec);
#endif
#ifdef TE_SUPPORT_METAL
    case GraphicsAPI::Metal:
        return CreateRef<MetalFramebuffer>(spec);
#endif
    default:
        break;
    }
    return nullptr;
}
