#include "Core/PreRequisites.h"
#include "Renderer/RendererAPI.hpp"
#include "Renderer/RendererContext.hpp"

#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLRendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_OPENGLES
#include "Renderer/OpenGLES/OpenGLESRendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_VULKAN
#include "Renderer/Vulkan/VulkanRendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_DIRECTX11
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#endif

#ifdef TE_SUPPORT_METAL
#include "Renderer/Metal/MetalRendererAPI.hpp"
#endif
GraphicsAPI RendererAPI::GetAPI() { return RendererContext::GetAPI(); }

TEScope<RendererAPI> RendererAPI::Create()
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::None:
        return nullptr;
    case GraphicsAPI::OpenGL:
#ifdef TE_SUPPORT_OPENGL
        return CreateScope<OpenGLRendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::OpenGLES:
#ifdef TE_SUPPORT_OPENGLES
        return CreateScope<OpenGLESRendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::Vulkan:
#ifdef TE_SUPPORT_VULKAN
        return CreateScope<VulkanRendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::DirectX11:
#ifdef TE_SUPPORT_DIRECTX11
        return CreateScope<DirectX11RendererAPI>();
#else
        return nullptr;
#endif
    case GraphicsAPI::Metal:
#ifdef TE_SUPPORT_METAL
        return CreateScope<MetalRendererAPI>();
#else
        return nullptr;
#endif
    }
    return nullptr;
}
