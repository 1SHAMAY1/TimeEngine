#pragma once
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif

    void *MetalCreateTexture2D(uint32_t width, uint32_t height, uint32_t channels, const void *data,
                               bool generateMipmaps);
    void MetalDestroyTexture(void *texture);

#ifdef __cplusplus
}
#endif
