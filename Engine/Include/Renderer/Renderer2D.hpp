#pragma once
#include "Renderer/Renderer.hpp"
#include "Renderer/RenderBatcher.hpp"
#include "Utility/MathUtils.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace TE {
    class LightComponent;

class Renderer2D : public Renderer {
public:
    static std::shared_ptr<Renderer2D> Create();
    Renderer2D();
    virtual ~Renderer2D() override;

    void BeginFrame(const glm::mat4& viewProjection) override;
    void Submit(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& transform, uint32_t indexCount) override;
    void EndFrame() override;
    void Flush() override;

    // 2D-specific API (example: submit a quad)
    void SubmitQuad(const TEVector2& position, const TEVector2& size, const std::shared_ptr<Material>& material);
    void SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Material>& material);
    void SubmitTriangle(const TEVector2& p1, const TEVector2& p2, const TEVector2& p3, const std::shared_ptr<Material>& material);
    void SubmitCircle(const TEVector2& center, float radius, const std::shared_ptr<Material>& material);
    void SubmitLight(const class LightComponent& light, const TEVector2& position);

    // Debug Outlines
    void SubmitRectOutline(const TEVector2& position, const TEVector2& size, float thickness, const TEColor& color);
    void SubmitCircleOutline(const TEVector2& center, float radius, float thickness, const TEColor& color);

private:
    RenderBatcher m_Batcher;
};

} 