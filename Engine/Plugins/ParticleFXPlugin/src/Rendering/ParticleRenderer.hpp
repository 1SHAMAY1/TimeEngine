#pragma once
#include "../Core/ParticlePool.hpp"
#include "../Core/ParticleTypes.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/Shader.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Utils/MathUtils.hpp"
#include <memory>

struct ParticleVertex
{
    glm::vec3 Position;
    glm::vec2 TexCoord;
    glm::vec4 Color;
};

class ParticleRenderer
{
public:
    ParticleRenderer(uint32_t maxParticles = 10000);
    ~ParticleRenderer();

    void Init();
    void Begin(const glm::mat4 &viewProjection);
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
    glm::mat4 m_ViewProjection = glm::mat4(1.0f);
};
