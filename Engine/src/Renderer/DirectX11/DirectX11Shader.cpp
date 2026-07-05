// Windows headers first to prevent macro pollution
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

// Engine headers after Windows/D3D11
#include "Renderer/DirectX11/DirectX11RendererAPI.hpp"
#include "Renderer/DirectX11/DirectX11Shader.hpp"
#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// ID3DBlob is typedef'd as ID3D10Blob in d3dcommon.h — use ID3D10Blob throughout
typedef ID3D10Blob ID3DBlob;

namespace TE
{

// -------------------------------------------------------------------------
//  Internal helper: compile HLSL source to bytecode
// -------------------------------------------------------------------------
static ID3D10Blob *CompileHLSL(const std::string &src, const char *entryPoint, const char *target)
{
    ID3D10Blob *codeBlob = nullptr;
    ID3D10Blob *errorBlob = nullptr;

    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG) || defined(TE_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    HRESULT hr = D3DCompile(src.c_str(), src.size(),
                            nullptr, // source name (used in error messages)
                            nullptr, // macros
                            nullptr, // includes
                            entryPoint, target, flags, 0, &codeBlob, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            std::cerr << "[DirectX11] Shader compile error (" << entryPoint << "):\n"
                      << static_cast<char *>(errorBlob->GetBufferPointer()) << "\n";
            errorBlob->Release();
        }
        else
        {
            std::cerr << "[DirectX11] Shader compile failed: 0x" << std::hex << hr << "\n";
        }
        return nullptr;
    }
    if (errorBlob)
        errorBlob->Release();
    return codeBlob;
}

// -------------------------------------------------------------------------
//  Constructor
// -------------------------------------------------------------------------
DirectX11Shader::DirectX11Shader(const std::string &vertexSrc, const std::string &fragmentSrc)
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.Device)
    {
        std::cerr << "[DirectX11] Shader: Device is null. Call InitWithWindow first.\n";
        return;
    }

    // --- Vertex shader ---
    m_VSBlob = CompileHLSL(vertexSrc, "VSMain", "vs_5_0");
    if (!m_VSBlob)
        return;

    HRESULT hr = ctx.Device->CreateVertexShader(m_VSBlob->GetBufferPointer(), m_VSBlob->GetBufferSize(), nullptr,
                                                &m_VertexShader);
    if (FAILED(hr))
    {
        std::cerr << "[DirectX11] CreateVertexShader failed: 0x" << std::hex << hr << "\n";
        return;
    }

    // --- Pixel shader ---
    ID3D10Blob *psBlob = CompileHLSL(fragmentSrc, "PSMain", "ps_5_0");
    if (psBlob)
    {
        ctx.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_PixelShader);
        psBlob->Release();
    }

    // --- Constant buffer (1024 bytes, must be a multiple of 16) ---
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.ByteWidth = sizeof(DX11ConstantBufferData); // 256 floats * 4 bytes = 1024
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    hr = ctx.Device->CreateBuffer(&cbDesc, nullptr, &m_ConstantBuffer);
    if (FAILED(hr))
        std::cerr << "[DirectX11] CreateBuffer (constant) failed: 0x" << std::hex << hr << "\n";
}

DirectX11Shader::~DirectX11Shader()
{
    if (m_ConstantBuffer)
    {
        m_ConstantBuffer->Release();
        m_ConstantBuffer = nullptr;
    }
    if (m_VSBlob)
    {
        m_VSBlob->Release();
        m_VSBlob = nullptr;
    }
    if (m_PixelShader)
    {
        m_PixelShader->Release();
        m_PixelShader = nullptr;
    }
    if (m_VertexShader)
    {
        m_VertexShader->Release();
        m_VertexShader = nullptr;
    }
}

// -------------------------------------------------------------------------
//  Bind / Unbind
// -------------------------------------------------------------------------
void DirectX11Shader::Bind() const
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.DeviceContext)
        return;

    ctx.DeviceContext->VSSetShader(m_VertexShader, nullptr, 0);
    ctx.DeviceContext->PSSetShader(m_PixelShader, nullptr, 0);

    if (m_Dirty)
        FlushConstantBuffer();

    if (m_ConstantBuffer)
    {
        ctx.DeviceContext->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
        ctx.DeviceContext->PSSetConstantBuffers(0, 1, &m_ConstantBuffer);
    }
}

void DirectX11Shader::Unbind() const
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.DeviceContext)
        return;
    ctx.DeviceContext->VSSetShader(nullptr, nullptr, 0);
    ctx.DeviceContext->PSSetShader(nullptr, nullptr, 0);
}

// -------------------------------------------------------------------------
//  Upload CPU-side constant buffer to GPU
// -------------------------------------------------------------------------
void DirectX11Shader::FlushConstantBuffer() const
{
    DX11Context &ctx = DX11Context::Get();
    if (!ctx.DeviceContext || !m_ConstantBuffer)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (SUCCEEDED(ctx.DeviceContext->Map(m_ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        std::memcpy(mapped.pData, &m_CBData, sizeof(m_CBData));
        ctx.DeviceContext->Unmap(m_ConstantBuffer, 0);
    }
    m_Dirty = false;
}

// -------------------------------------------------------------------------
//  Uniform helpers — pack data into flat float array, respecting HLSL alignment
// -------------------------------------------------------------------------
static uint32_t AlignOffset(uint32_t floatOffset, uint32_t floatsNeeded)
{
    // D3D11 constant buffer packing: a value must not straddle a 16-byte (4-float) boundary.
    uint32_t boundary = (floatOffset / 4) * 4;
    if (floatOffset != boundary && (floatOffset % 4) + floatsNeeded > 4)
        floatOffset = boundary + 4;
    return floatOffset;
}

void DirectX11Shader::SetUniformMat4(const std::string &name, const glm::mat4 &value)
{
    auto it = m_UniformOffsets.find(name);
    uint32_t offset;
    if (it == m_UniformOffsets.end())
    {
        offset = AlignOffset(m_NextOffset, 16);
        m_UniformOffsets[name] = offset;
        m_NextOffset = offset + 16;
    }
    else
    {
        offset = it->second;
    }
    if (offset + 16 <= 256)
        std::memcpy(&m_CBData.data[offset], glm::value_ptr(value), 16 * sizeof(float));
    m_Dirty = true;
}

void DirectX11Shader::SetUniform4f(const std::string &name, const glm::vec4 &value)
{
    auto it = m_UniformOffsets.find(name);
    uint32_t offset;
    if (it == m_UniformOffsets.end())
    {
        offset = AlignOffset(m_NextOffset, 4);
        m_UniformOffsets[name] = offset;
        m_NextOffset = offset + 4;
    }
    else
    {
        offset = it->second;
    }
    if (offset + 4 <= 256)
        std::memcpy(&m_CBData.data[offset], glm::value_ptr(value), 4 * sizeof(float));
    m_Dirty = true;
}

void DirectX11Shader::SetUniform3f(const std::string &name, const glm::vec3 &value)
{
    auto it = m_UniformOffsets.find(name);
    uint32_t offset;
    if (it == m_UniformOffsets.end())
    {
        offset = AlignOffset(m_NextOffset, 3);
        m_UniformOffsets[name] = offset;
        m_NextOffset = offset + 4; // vec3 occupies a full 4-float slot in HLSL cbuffer
    }
    else
    {
        offset = it->second;
    }
    if (offset + 3 <= 256)
        std::memcpy(&m_CBData.data[offset], glm::value_ptr(value), 3 * sizeof(float));
    m_Dirty = true;
}

void DirectX11Shader::SetUniform2f(const std::string &name, const glm::vec2 &value)
{
    auto it = m_UniformOffsets.find(name);
    uint32_t offset;
    if (it == m_UniformOffsets.end())
    {
        offset = AlignOffset(m_NextOffset, 2);
        m_UniformOffsets[name] = offset;
        m_NextOffset = offset + 2;
    }
    else
    {
        offset = it->second;
    }
    if (offset + 2 <= 256)
        std::memcpy(&m_CBData.data[offset], glm::value_ptr(value), 2 * sizeof(float));
    m_Dirty = true;
}

void DirectX11Shader::SetUniform1f(const std::string &name, float value)
{
    auto it = m_UniformOffsets.find(name);
    uint32_t offset;
    if (it == m_UniformOffsets.end())
    {
        offset = m_NextOffset++;
        m_UniformOffsets[name] = offset;
    }
    else
    {
        offset = it->second;
    }
    if (offset < 256)
        m_CBData.data[offset] = value;
    m_Dirty = true;
}

void DirectX11Shader::SetUniform1i(const std::string &name, int value)
{
    // Texture slot bindings are handled via PSSetShaderResources in D3D11 (not uniforms).
    // We still record this for interface compatibility.
    auto it = m_UniformOffsets.find(name);
    uint32_t offset;
    if (it == m_UniformOffsets.end())
    {
        offset = m_NextOffset++;
        m_UniformOffsets[name] = offset;
    }
    else
    {
        offset = it->second;
    }
    if (offset < 256)
        m_CBData.data[offset] = static_cast<float>(value);
    m_Dirty = true;
}

} // namespace TE
