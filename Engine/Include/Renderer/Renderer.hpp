#pragma once
#include <glm/glm.hpp>
#include <memory>
#include "Renderer/VertexArray.hpp"
#include "Renderer/Material.hpp"

namespace TE {

class Renderer {
public:
    virtual ~Renderer() = default;
    virtual void BeginFrame() = 0;
    virtual void Submit(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& transform, uint32_t indexCount) = 0;
    virtual void EndFrame() = 0;
    virtual void Flush() = 0;
};

}
