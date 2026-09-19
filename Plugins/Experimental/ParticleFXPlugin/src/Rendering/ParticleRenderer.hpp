#pragma once
#include "../Core/ParticlePool.hpp"
#include "../Core/ParticleTypes.hpp"
#include "IndexBuffer.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include "MathUtils.hpp"

struct ParticleVertex
{
    TEVector Position;
    TEVector2 TexCoord;
    TEVector4 Color;
};

class ParticleRenderer
{
public:
    ParticleRenderer(uint32_t maxParticles = 10000);
    ~ParticleRenderer();

    void Init();
    void Begin(const TEMatrix4 &viewProjection);
    void Render(const ParticlePool &pool, EParticleBlendMode blendMode = EParticleBlendMode::AlphaBlend,
                const TERef<Texture2D> &texture = nullptr);
    void End();

private:
    void Flush();

    uint32_t m_MaxParticles = 10000;
    uint32_t m_MaxVertices = 40000;
    uint32_t m_MaxIndices = 60000;

    TERef<VertexArray> m_VAO;
    TERef<VertexBuffer> m_VBO;
    TERef<IndexBuffer> m_IBO;
    TERef<Shader> m_Shader;

    TEArray<ParticleVertex> m_VertexBufferBase;
    uint32_t m_IndexCount = 0;
    TEMatrix4 m_ViewProjection = TEMatrix4(1.0f);
};
