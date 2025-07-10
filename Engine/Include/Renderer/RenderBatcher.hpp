#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "Renderer/Material.hpp"
#include "Renderer/VertexArray.hpp"

namespace TE {

struct BatchDrawCommand {
    std::shared_ptr<VertexArray> vertexArray;
    std::shared_ptr<Material> material;
    glm::mat4 transform;
    uint32_t indexCount;
};

class RenderBatcher {
public:
    void Begin();
    void Submit(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& transform, uint32_t indexCount);
    void End();
    void Flush(); // Issues the actual draw calls, batching by material/shader

private:
    std::vector<BatchDrawCommand> m_DrawCommands;
};

} 