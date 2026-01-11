#include "Renderer/Renderer2D.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/IndexBuffer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Utility/MathUtils.hpp"

namespace TE {

Renderer2D::Renderer2D() {}
Renderer2D::~Renderer2D() {}

void Renderer2D::BeginFrame() {
    m_Batcher.Begin();
}

void Renderer2D::Submit(const std::shared_ptr<VertexArray>& vao, const std::shared_ptr<Material>& material, const glm::mat4& transform, uint32_t indexCount) {
    m_Batcher.Submit(vao, material, transform, indexCount);
}

void Renderer2D::EndFrame() {
    m_Batcher.End();
}

void Renderer2D::Flush() {
    m_Batcher.Flush();
}

void Renderer2D::SubmitQuad(const glm::vec2& position, const glm::vec2& size, const std::shared_ptr<Material>& material) {
    float x = position.x, y = position.y, w = size.x, h = size.y;
    float vertices[] = {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };
    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
    auto vao = std::shared_ptr<VertexArray>(VertexArray::Create());
    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    auto ibo = IndexBuffer::Create(indices, 6);
    vao->Bind();
    vbo->Bind();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    TETransform transform;
    transform.Position = glm::vec3(x, y, 0.0f);
    transform.Scale = TEScale(w, h, 1.0f);
    m_Batcher.Submit(vao, material, transform.GetMatrix(), 6);
}

std::shared_ptr<Renderer2D> Renderer2D::Create() {
    return std::make_shared<Renderer2D>();
}

} // namespace TE