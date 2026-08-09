#include "Renderer/Metal/MetalShader.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#endif

namespace TE
{

MetalShader::MetalShader(const std::string &vertexSrc, const std::string &fragmentSrc)
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device)
    {
        NSString *src = [NSString stringWithUTF8String:(vertexSrc + "\n" + fragmentSrc).c_str()];
        NSError *error = nil;
        id<MTLLibrary> lib = [device newLibraryWithSource:src options:nil error:&error];
        if (lib)
        {
            m_Library = (__bridge_retained void *)lib;
        }
        else if (error)
        {
            TE_CORE_WARN("MetalShader compilation warning: {0}", [error.localizedDescription UTF8String]);
        }
    }
#endif
}

MetalShader::MetalShader(const std::string &computeSrc)
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device)
    {
        NSString *src = [NSString stringWithUTF8String:computeSrc.c_str()];
        NSError *error = nil;
        id<MTLLibrary> lib = [device newLibraryWithSource:src options:nil error:&error];
        if (lib)
        {
            m_Library = (__bridge_retained void *)lib;
        }
    }
#endif
}

MetalShader::~MetalShader()
{
#ifdef TE_SUPPORT_METAL
    if (m_Library)
    {
        id<MTLLibrary> lib = (__bridge_transfer id<MTLLibrary>)m_Library;
        lib = nil;
        m_Library = nullptr;
    }
#endif
}

void MetalShader::Bind() const
{
}

void MetalShader::Unbind() const
{
}

void MetalShader::SetUniformMat4(const std::string &name, const glm::mat4 &value)
{
}

void MetalShader::SetUniform4f(const std::string &name, const glm::vec4 &value)
{
}

void MetalShader::SetUniform3f(const std::string &name, const glm::vec3 &value)
{
}

void MetalShader::SetUniform2f(const std::string &name, const glm::vec2 &value)
{
}

void MetalShader::SetUniform1f(const std::string &name, float value)
{
}

void MetalShader::SetUniform1i(const std::string &name, int value)
{
}

} // namespace TE
