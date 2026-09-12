#include "ParticleRenderer.hpp"
#include "ParticleShaders.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/Texture.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Utils/Math/MathEngine.hpp"

ParticleRenderer::ParticleRenderer(uint32_t maxParticles)
    : m_MaxParticles(maxParticles), m_MaxVertices(maxParticles * 4), m_MaxIndices(maxParticles * 6)
{
    m_VertexBufferBase.Reserve(m_MaxVertices);
}

ParticleRenderer::~ParticleRenderer() {}

void ParticleRenderer::Init()
{
    if (m_VAO)
        return;

    m_Shader = Shader::Create(ParticleShaders::GetVertexShader(), ParticleShaders::GetFragmentShader());

    m_VAO = VertexArray::Create();
    m_VBO = VertexBuffer::Create(nullptr, m_MaxVertices * sizeof(ParticleVertex));
    m_VAO->AddVertexBuffer(m_VBO);

    // Build index buffer for quads
    TEArray<uint32_t> indices;
    indices.Resize(m_MaxIndices, 0);
    uint32_t offset = 0;
    for (uint32_t i = 0; i < m_MaxIndices; i += 6)
    {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;

        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;

        offset += 4;
    }

    m_IBO = IndexBuffer::Create(indices.Data(), m_MaxIndices);
    m_VAO->SetIndexBuffer(m_IBO);
}

void ParticleRenderer::Begin(const TEMatrix4 &viewProjection)
{
    Init();
    m_ViewProjection = viewProjection;
    m_VertexBufferBase.Clear();
    m_IndexCount = 0;
}

void ParticleRenderer::Render(const ParticlePool &pool, EParticleBlendMode blendMode, const TERef<Texture2D> &texture)
{
    // Apply Blend Mode
    if (blendMode == EParticleBlendMode::Additive)
    {
        RenderCommand::SetBlendMode(1); // Additive
    }
    else if (blendMode == EParticleBlendMode::AlphaBlend)
    {
        RenderCommand::SetBlendMode(0); // Alpha Blend
    }

    // Quad vertex offsets
    static const TEVector4 quadPositions[4] = {
        {-0.5f, -0.5f, 0.0f, 1.0f}, {0.5f, -0.5f, 0.0f, 1.0f}, {0.5f, 0.5f, 0.0f, 1.0f}, {-0.5f, 0.5f, 0.0f, 1.0f}};

    static const TEVector2 texCoords[4] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

    for (const auto &p : pool.GetParticles())
    {
        if (!p.Active)
            continue;

        if (m_IndexCount >= m_MaxIndices)
            Flush();

        TEMatrix4 translation = MathEngine::Get().GetActiveAPI()->Translate(
            TEMatrix4(1.0f), TEVector(p.Position.x, p.Position.y, p.Position.z));
        TEMatrix4 rotation =
            MathEngine::Get().GetActiveAPI()->Rotate(TEMatrix4(1.0f), p.Rotation, TEVector(0.0f, 0.0f, 1.0f));
        TEMatrix4 scale = MathEngine::Get().GetActiveAPI()->Scale(TEMatrix4(1.0f), TEVector(p.Size, p.Size, 1.0f));
        TEMatrix4 transform = translation * rotation * scale;

        TEVector4 pColor(p.Color.x, p.Color.y, p.Color.z, p.Color.w);

        for (int i = 0; i < 4; ++i)
        {
            ParticleVertex vertex;
            TEVector4 transformedPos = transform * quadPositions[i];
            vertex.Position = TEVector(transformedPos.x, transformedPos.y, transformedPos.z);
            vertex.TexCoord = texCoords[i];
            vertex.Color = pColor;
            m_VertexBufferBase.Add(vertex);
        }

        m_IndexCount += 6;
    }

    if (texture)
    {
        texture->Bind(0);
        m_Shader->Bind();
        m_Shader->SetUniform1i("u_UseTexture", 1);
        m_Shader->SetUniform1i("u_Texture", 0);
    }
    else
    {
        m_Shader->Bind();
        m_Shader->SetUniform1i("u_UseTexture", 0);
    }

    Flush();
}

void ParticleRenderer::Flush()
{
    if (m_IndexCount == 0 || m_VertexBufferBase.IsEmpty())
        return;

    m_Shader->Bind();
    m_Shader->SetUniformMat4("u_ViewProjection", m_ViewProjection);

    m_VBO->Bind();
    m_VBO->SetData(reinterpret_cast<float *>(m_VertexBufferBase.Data()),
                   (uint32_t)(m_VertexBufferBase.Num() * sizeof(ParticleVertex)));
    m_VAO->Bind();
    RenderCommand::DrawIndexed(m_VAO->GetRendererID(), m_IndexCount);

    m_VertexBufferBase.Clear();
    m_IndexCount = 0;
}

void ParticleRenderer::End()
{
    Flush();
    RenderCommand::SetBlendMode(0); // Reset to Alpha Blend
}
