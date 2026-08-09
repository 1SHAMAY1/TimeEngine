#include "Renderer/Metal/MetalVertexBuffer.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#endif

namespace TE
{

MetalVertexBuffer::MetalVertexBuffer(float *vertices, uint32_t size)
    : m_Size(size)
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device && vertices && size > 0)
    {
        id<MTLBuffer> buffer = [device newBufferWithBytes:vertices
                                                  length:size
                                                 options:MTLResourceStorageModeShared];
        m_Buffer = (__bridge_retained void *)buffer;
    }
#endif
}

MetalVertexBuffer::~MetalVertexBuffer()
{
#ifdef TE_SUPPORT_METAL
    if (m_Buffer)
    {
        id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)m_Buffer;
        buffer = nil;
        m_Buffer = nullptr;
    }
#endif
}

void MetalVertexBuffer::Bind() const
{
}

void MetalVertexBuffer::Unbind() const
{
}

void MetalVertexBuffer::SetData(float *vertices, uint32_t size) const
{
#ifdef TE_SUPPORT_METAL
    if (m_Buffer && vertices && size > 0)
    {
        id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)m_Buffer;
        memcpy([buffer contents], vertices, size);
    }
#endif
}

} // namespace TE
