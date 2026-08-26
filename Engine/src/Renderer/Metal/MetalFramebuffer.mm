#include "Renderer/Metal/MetalFramebuffer.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#endif

MetalFramebuffer::MetalFramebuffer(const FramebufferSpecification &spec)
    : m_Specification(spec)
{
    Invalidate();
}

MetalFramebuffer::~MetalFramebuffer()
{
#ifdef TE_SUPPORT_METAL
    if (m_ColorTexture)
    {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)m_ColorTexture;
        tex = nil;
        m_ColorTexture = nullptr;
    }
    if (m_DepthTexture)
    {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)m_DepthTexture;
        tex = nil;
        m_DepthTexture = nullptr;
    }
#endif
}

void MetalFramebuffer::Invalidate()
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device || m_Specification.Width == 0 || m_Specification.Height == 0)
        return;

    MTLTextureDescriptor *colorDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:m_Specification.Width
                                                                                         height:m_Specification.Height
                                                                                      mipmapped:NO];
    colorDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
    id<MTLTexture> colorTex = [device newTextureWithDescriptor:colorDesc];
    m_ColorTexture = (__bridge_retained void *)colorTex;
#endif
}

void MetalFramebuffer::Bind()
{
}

void MetalFramebuffer::Unbind()
{
}

void MetalFramebuffer::Resize(uint32_t width, uint32_t height)
{
    m_Specification.Width = width;
    m_Specification.Height = height;
    Invalidate();
}
