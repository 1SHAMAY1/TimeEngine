#pragma once
// d3dcommon.h is lightweight (no device creation) and correctly defines ID3DBlob/ID3D10Blob.
// Using it here avoids the "redefinition; different basic types" error caused by
// the incorrect 'struct ID3DBlob;' forward declaration.
#include "Renderer/Shader.hpp"
#include <d3dcommon.h>
#include <string>
#include <unordered_map>

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Buffer;

namespace TE
{

// Per-draw constant buffer layout that mirrors GLSL uniforms.
// All members must be aligned to 16 bytes as required by D3D11 constant buffers.
struct DX11ConstantBufferData
{
    float data[256] = {}; // 256 floats = 1024 bytes, covers most shader uniforms
};

class DirectX11Shader : public Shader
{
public:
    DirectX11Shader(const std::string &vertexSrc, const std::string &fragmentSrc);
    virtual ~DirectX11Shader();

    virtual void Bind() const override;
    virtual void Unbind() const override;

    virtual void SetUniformMat4(const std::string &name, const glm::mat4 &value) override;
    virtual void SetUniform4f(const std::string &name, const glm::vec4 &value) override;
    virtual void SetUniform3f(const std::string &name, const glm::vec3 &value) override;
    virtual void SetUniform2f(const std::string &name, const glm::vec2 &value) override;
    virtual void SetUniform1f(const std::string &name, float value) override;
    virtual void SetUniform1i(const std::string &name, int value) override;

    // Expose the VS bytecode blob so that DirectX11VertexArray can build an InputLayout
    ID3D10Blob *GetVSBlob() const { return m_VSBlob; }

private:
    void FlushConstantBuffer() const;

    ID3D11VertexShader *m_VertexShader = nullptr;
    ID3D11PixelShader *m_PixelShader = nullptr;
    ID3D11Buffer *m_ConstantBuffer = nullptr;
    ID3D10Blob *m_VSBlob = nullptr; // ID3DBlob == ID3D10Blob typedef

    mutable DX11ConstantBufferData m_CBData;
    mutable std::unordered_map<std::string, uint32_t> m_UniformOffsets;
    mutable uint32_t m_NextOffset = 0;
    mutable bool m_Dirty = false;
};

} // namespace TE
