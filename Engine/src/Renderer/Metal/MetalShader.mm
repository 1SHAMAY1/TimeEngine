#include "Renderer/Metal/MetalShader.hpp"
#include "Core/Log.h"

#ifdef TE_SUPPORT_METAL
#import <Metal/Metal.h>
#endif

MetalShader::MetalShader(const TEString &vertexSrc, const TEString &fragmentSrc)
{
#ifdef TE_SUPPORT_METAL
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    if (device)
    {
        NSError *error = nil;
        id<MTLLibrary> lib = nil;

        // If source does not contain GLSL preprocessor directive #version, attempt to compile as MSL
        if (vertexSrc.Find("#version") == TEString::npos && fragmentSrc.Find("#version") == TEString::npos)
        {
            TEString combined = vertexSrc + "\n" + fragmentSrc;
            NSString *src = [NSString stringWithUTF8String:combined.c_str()];
            lib = [device newLibraryWithSource:src options:nil error:&error];
        }

        // Standard 2D MSL shader fallback
        if (!lib)
        {
            NSString *mslSrc = @R"(
#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    float4 color [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
    float texIndex [[attribute(3)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
    float2 texCoord;
    float texIndex;
};

struct Uniforms {
    float4x4 u_ViewProjection;
};

vertex VertexOut vertexMain(VertexIn in [[stage_in]], constant Uniforms& uniforms [[buffer(1)]]) {
    VertexOut out;
    out.position = uniforms.u_ViewProjection * float4(in.position, 1.0);
    out.color = in.color;
    out.texCoord = in.texCoord;
    out.texIndex = in.texIndex;
    return out;
}

fragment float4 fragmentMain(VertexOut in [[stage_in]], texture2d<float> u_Texture [[texture(0)]], sampler u_Sampler [[sampler(0)]]) {
    return in.color * u_Texture.sample(u_Sampler, in.texCoord);
}
)";
            error = nil;
            lib = [device newLibraryWithSource:mslSrc options:nil error:&error];
        }

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

MetalShader::MetalShader(const TEString &computeSrc)
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

void MetalShader::SetUniformMat4(const TEString &name, const glm::mat4 &value)
{
}

void MetalShader::SetUniform4f(const TEString &name, const glm::vec4 &value)
{
}

void MetalShader::SetUniform3f(const TEString &name, const glm::vec3 &value)
{
}

void MetalShader::SetUniform2f(const TEString &name, const glm::vec2 &value)
{
}

void MetalShader::SetUniform1f(const TEString &name, float value)
{
}

void MetalShader::SetUniform1i(const TEString &name, int value)
{
}
