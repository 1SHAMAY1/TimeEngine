#pragma once
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderBatcher.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace TE {

class Renderer2D : public Renderer {
public:
    static std::shared_ptr<Renderer2D> Create();
    Renderer2D();
    virtual ~Renderer2D() override;

    void BeginFrame() override;
    void Submit(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& transform, uint32_t indexCount) override;
    void EndFrame() override;
    void Flush() override;

    // 2D-specific API (example: submit a quad)
    void SubmitQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Material>& material);

private:
    RenderBatcher m_Batcher;
};

} 