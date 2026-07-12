#include "Renderer/RenderBatcher.hpp"
#include "Layers/ProfilingLayer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include <chrono>

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
    auto startTime = std::chrono::high_resolution_clock::now();

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
    RenderCommand::SetBlendMode(0);

    uint32_t totalDrawCalls = 0;
    uint32_t totalTriangles = 0;
    uint32_t totalVertices = 0;

    for (const auto &cmd : m_DrawCommands)
    {
        if (cmd.blendMode != lastBlendMode)
        {
            RenderCommand::SetBlendMode(cmd.blendMode);
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

        totalDrawCalls++;
        totalTriangles += cmd.indexCount / 3;
        totalVertices += cmd.indexCount;
    }

    // Reset to default
    RenderCommand::SetBlendMode(0);
    m_DrawCommands.clear();

    auto endTime = std::chrono::high_resolution_clock::now();
    float durationMs = std::chrono::duration<float, std::milli>(endTime - startTime).count();

    if (auto *profiler = ProfilingLayer::GetInstance())
    {
        profiler->RecordRenderTime(durationMs);
        for (uint32_t i = 0; i < totalDrawCalls; ++i)
        {
            profiler->RecordDrawCall();
        }
        profiler->RecordTriangle(totalTriangles);
        profiler->RecordVertex(totalVertices);
    }
}

} // namespace TE