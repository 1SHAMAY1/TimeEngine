#include "Renderer/Metal/MetalIndexBuffer.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#endif

MetalIndexBuffer::MetalIndexBuffer(uint32_t *indices, uint32_t count)
    : m_Count(count)
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device && indices && count > 0)
    {
        uint32_t size = count * sizeof(uint32_t);
        id<MTLBuffer> buffer = [device newBufferWithBytes:indices
                                                  length:size
                                                 options:MTLResourceStorageModeShared];
        m_Buffer = (__bridge_retained void *)buffer;
    }
#endif
}

MetalIndexBuffer::~MetalIndexBuffer()
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

void MetalIndexBuffer::Bind() const
{
}

void MetalIndexBuffer::Unbind() const
{
}

void MetalIndexBuffer::SetData(uint32_t *indices, uint32_t count) const
{
#ifdef TE_SUPPORT_METAL
    if (m_Buffer && indices && count > 0)
    {
        id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)m_Buffer;
        memcpy([buffer contents], indices, count * sizeof(uint32_t));
    }
#endif
}
