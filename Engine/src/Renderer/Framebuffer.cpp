#include "Renderer/Framebuffer.hpp"
#ifdef TE_SUPPORT_OPENGL
#include "Renderer/OpenGL/OpenGLFramebuffer.hpp"
#endif
#include "Renderer/RendererContext.hpp"

#if defined(TE_PLATFORM_MOBILE)
#include "Renderer/OpenGLES/OpenGLESFramebuffer.hpp"
#endif

#ifdef TE_SUPPORT_METAL
#include "Renderer/Metal/MetalFramebuffer.hpp"
#endif

namespace TE
{

std::shared_ptr<Framebuffer> Framebuffer::Create(const FramebufferSpecification &spec)
{
    switch (RendererContext::GetAPI())
    {
    case GraphicsAPI::None:
        return nullptr;
#ifdef TE_SUPPORT_OPENGL
    case GraphicsAPI::OpenGL:
        return std::make_shared<OpenGLFramebuffer>(spec);
#endif
#if defined(TE_PLATFORM_MOBILE)
    case GraphicsAPI::OpenGLES:
        return std::make_shared<OpenGLESFramebuffer>(spec);
#endif
#ifdef TE_SUPPORT_METAL
    case GraphicsAPI::Metal:
        return std::make_shared<MetalFramebuffer>(spec);
#endif
    default:
        break;
    }
    return nullptr;
}

} // namespace TE
