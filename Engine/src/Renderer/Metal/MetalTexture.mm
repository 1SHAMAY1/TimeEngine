#include "Renderer/Metal/MetalTexture.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#endif

extern "C" {

void *MetalCreateTexture2D(uint32_t width, uint32_t height, uint32_t channels, const void *data, bool generateMipmaps)
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (!device || width == 0 || height == 0 || !data)
        return nullptr;

    MTLPixelFormat pixelFormat = MTLPixelFormatRGBA8Unorm;
    if (channels == 1)
        pixelFormat = MTLPixelFormatR8Unorm;
    else if (channels == 2)
        pixelFormat = MTLPixelFormatRG8Unorm;
    else if (channels == 4)
        pixelFormat = MTLPixelFormatRGBA8Unorm;

    MTLTextureDescriptor *desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:pixelFormat
                                                                                    width:width
                                                                                   height:height
                                                                                mipmapped:generateMipmaps ? YES : NO];
    desc.usage = MTLTextureUsageShaderRead;

    id<MTLTexture> texture = [device newTextureWithDescriptor:desc];
    if (!texture)
    {
        TE_CORE_ERROR("Metal: Failed to create texture with dimensions ({0}, {1})", width, height);
        return nullptr;
    }

    NSUInteger bytesPerRow = width * channels;
    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerRow];

    return (__bridge_retained void *)texture;
#else
    return nullptr;
#endif
}

void MetalDestroyTexture(void *texture)
{
#ifdef TE_SUPPORT_METAL
    if (texture)
    {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)texture;
        tex = nil;
    }
#endif
}

}
