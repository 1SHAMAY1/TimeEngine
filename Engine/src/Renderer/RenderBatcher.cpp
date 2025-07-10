#include "Renderer/RenderBatcher.hpp"
#include "Renderer/RenderCommand.hpp"
#include "Renderer/ShaderLibrary.hpp"

namespace TE {

void RenderBatcher::Begin() {
    m_DrawCommands.clear();
}

void RenderBatcher::Submit(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& transform, uint32_t indexCount) {
    m_DrawCommands.push_back({vao, material, transform, indexCount});
}

void RenderBatcher::End() {
    // No-op for now
}

void RenderBatcher::Flush() {
    // Simple batching: sort by material (shader pointer)
    std::sort(m_DrawCommands.begin(), m_DrawCommands.end(), [](const BatchDrawCommand& a, const BatchDrawCommand& b) {
        return a.material->GetShader().get() < b.material->GetShader().get();
    });

    std::shared_ptr<Material> lastMaterial = nullptr;
    for (const auto& cmd : m_DrawCommands) {
        if (!lastMaterial || cmd.material != lastMaterial) {
            cmd.material->GetShader()->Bind();
            cmd.material->ApplyUniforms();
            lastMaterial = cmd.material;
        }
        // Set transform uniform
        ShaderLibrary::SetTransform(cmd.material->GetShader().get(), cmd.transform);
        cmd.vertexArray->Bind();
        RenderCommand::DrawIndexed(cmd.vertexArray->GetRendererID(), cmd.indexCount);
    }
    m_DrawCommands.clear();
}

} // namespace TE 