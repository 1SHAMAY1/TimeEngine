#include "Renderer/RenderBatcher.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include <glad/glad.h>

namespace TE
{

void RenderBatcher::Begin() { m_DrawCommands.clear(); }

void RenderBatcher::Submit(const std::shared_ptr<VertexArray> &vao, const std::shared_ptr<Material> &material,
                           const glm::mat4 &transform, uint32_t indexCount, int blendMode)
{
    m_DrawCommands.push_back({vao, material, transform, indexCount, blendMode});
}

void RenderBatcher::End()
{
    // No-op for now
}

void RenderBatcher::Flush()
{
    // Simple batching: sort by blendMode then material (shader pointer)
    std::sort(m_DrawCommands.begin(), m_DrawCommands.end(),
              [](const BatchDrawCommand &a, const BatchDrawCommand &b)
              {
                  if (a.blendMode != b.blendMode)
                      return a.blendMode < b.blendMode;
                  return a.material->GetShader().get() < b.material->GetShader().get();
              });

    std::shared_ptr<Material> lastMaterial = nullptr;
    int lastBlendMode = 0;

    // Default blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (const auto &cmd : m_DrawCommands)
    {
        if (cmd.blendMode != lastBlendMode)
        {
            if (cmd.blendMode == 1) // Additive
            {
                glBlendFunc(GL_ONE, GL_ONE);
                glDisable(GL_DEPTH_TEST);
            }
            else if (cmd.blendMode == 2) // Multiplicative
            {
                glBlendFunc(GL_DST_COLOR, GL_ZERO);
                glDisable(GL_DEPTH_TEST);
            }
            else // Normal
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_DEPTH_TEST);
            }
            lastBlendMode = cmd.blendMode;
        }

        if (!lastMaterial || cmd.material != lastMaterial)
        {
            cmd.material->GetShader()->Bind();
            cmd.material->ApplyUniforms();
            ShaderLibrary::SetViewProjection(cmd.material->GetShader().get(), m_ViewProjection);
            lastMaterial = cmd.material;
        }
        // Set transform uniform
        ShaderLibrary::SetTransform(cmd.material->GetShader().get(), cmd.transform);
        cmd.vertexArray->Bind();
        RenderCommand::DrawIndexed(cmd.vertexArray->GetRendererID(), cmd.indexCount);
    }

    // Reset to default
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    m_DrawCommands.clear();
}

} // namespace TE