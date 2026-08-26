#include "Core/PreRequisites.h"
#include "Renderer/RenderBatcher.hpp"
#include "Layers/ProfilingLayer.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include <chrono>

void RenderBatcher::Begin() { m_DrawCommands.Empty(); }

void RenderBatcher::Submit(const TERef<VertexArray> &vao, const TERef<Material> &material, const glm::mat4 &transform,
                           uint32_t indexCount, int blendMode, const TEColor &color)
{
    m_DrawCommands.Add({vao, material, transform, color, indexCount, blendMode});
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

    TERef<Material> lastMaterial = nullptr;
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
        // Set transform and color uniforms directly per draw command
        ShaderLibrary::SetTransform(cmd.material->GetShader().get(), cmd.transform);
        ShaderLibrary::SetColor(cmd.material->GetShader().get(), cmd.color);
        cmd.vertexArray->Bind();
        RenderCommand::DrawIndexed(cmd.vertexArray->GetRendererID(), cmd.indexCount);

        totalDrawCalls++;
        totalTriangles += cmd.indexCount / 3;
        totalVertices += cmd.indexCount;
    }

    // Reset to default
    RenderCommand::SetBlendMode(0);
    m_DrawCommands.Empty();

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
