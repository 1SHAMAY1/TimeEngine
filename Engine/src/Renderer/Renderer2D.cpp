#include "Renderer/Renderer2D.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Renderer/IndexBuffer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "Utility/MathUtils.hpp"
#include "Core/Scene/LightComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/ShaderLibrary.hpp"

namespace TE {

Renderer2D::Renderer2D() {}
Renderer2D::~Renderer2D() {}

void Renderer2D::BeginFrame(const glm::mat4& viewProjection) {
    m_Batcher.SetViewProjection(viewProjection);
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

void Renderer2D::SubmitQuad(const TEVector2& position, const TEVector2& size, const std::shared_ptr<Material>& material) {
    TETransform transform;
    transform.Position = glm::vec3(position.x, position.y, 0.0f);
    transform.Scale = TEScale(size.x, size.y, 1.0f);
    SubmitQuad(transform.GetMatrix(), material);
}

void Renderer2D::SubmitQuad(const glm::mat4& transform, const std::shared_ptr<Material>& material) {
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    uint32_t indices[] = { 0, 1, 2, 2, 3, 0 };
    auto vao = std::shared_ptr<VertexArray>(VertexArray::Create());
    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    auto ibo = IndexBuffer::Create(indices, 6);
    vao->Bind();
    vbo->Bind();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    m_Batcher.Submit(vao, material, transform, 6);
}

void Renderer2D::SubmitTriangle(const TEVector2& p1, const TEVector2& p2, const TEVector2& p3, const std::shared_ptr<Material>& material) {
    float vertices[] = {
        p1.x, p1.y, 0.0f,
        p2.x, p2.y, 0.0f,
        p3.x, p3.y, 0.0f
    };
    uint32_t indices[] = { 0, 1, 2 };
    auto vao = std::shared_ptr<VertexArray>(VertexArray::Create());
    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    auto ibo = IndexBuffer::Create(indices, 3);
    vao->Bind();
    vbo->Bind();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    m_Batcher.Submit(vao, material, glm::mat4(1.0f), 3);
}

void Renderer2D::SubmitCircle(const TEVector2& center, float radius, const std::shared_ptr<Material>& material) {
    const int segments = 32;
    std::vector<float> vertices;
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    vertices.push_back(0.0f);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * 3.14159265f * i / segments;
        vertices.push_back(center.x + std::cos(angle) * radius);
        vertices.push_back(center.y + std::sin(angle) * radius);
        vertices.push_back(0.0f);
    }
    std::vector<uint32_t> indices;
    for (int i = 1; i <= segments; ++i) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }
    auto vao = std::shared_ptr<VertexArray>(VertexArray::Create());
    auto vbo = VertexBuffer::Create(vertices.data(), (uint32_t)(vertices.size() * sizeof(float)));
    auto ibo = IndexBuffer::Create(indices.data(), (uint32_t)indices.size());
    vao->Bind();
    vbo->Bind();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    m_Batcher.Submit(vao, material, glm::mat4(1.0f), (uint32_t)indices.size());
}

void Renderer2D::SubmitLight(const LightComponent& light, const TEVector2& position) {
    // Visualize lights with simple shapes (icons or debug visuals)
    auto debugMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
    TEColor color = light.Color;
    debugMaterial->SetColor(color);

    if (light.Type == TELightType::Point) {
        // Point center
        SubmitCircle(position, 0.1f, debugMaterial);
        
        // Gradual fade effect using concentric rings
        const int fadeSteps = 5;
        for (int i = 1; i <= fadeSteps; i++) {
            float t = (float)i / fadeSteps;
            float stepAlpha = (1.0f - t) * 0.4f * light.Intensity;
            float stepRadius = light.Radius * t;

            TEColor stepColor(color.GetValue().r, color.GetValue().g, color.GetValue().b, stepAlpha);
            auto stepMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
            stepMaterial->SetColor(stepColor);
            SubmitCircle(position, stepRadius, stepMaterial);
        }
    }
    else if (light.Type == TELightType::Spot) {
        // Spot center
        SubmitCircle(position, 0.1f, debugMaterial);
        
        // Visualizing cone (Simplified)
        TEVector2 dir = light.Direction.Normalized();
        float outerRad = glm::radians(light.OuterAngle);
        // Drawing a triangle for the cone
        TEVector2 p1 = position;
        TEVector2 p2 = {position.x + (dir.x * cos(outerRad) - dir.y * sin(outerRad)) * light.Radius, 
                        position.y + (dir.x * sin(outerRad) + dir.y * cos(outerRad)) * light.Radius};
        TEVector2 p3 = {position.x + (dir.x * cos(-outerRad) - dir.y * sin(-outerRad)) * light.Radius,
                        position.y + (dir.x * sin(-outerRad) + dir.y * cos(-outerRad)) * light.Radius};
        
        // Gradual fade for spot light
        const int fadeSteps = 5;
        for (int i = 1; i <= fadeSteps; i++) {
            float t = (float)i / fadeSteps;
            float stepAlpha = (1.0f - t) * 0.4f * light.Intensity;
            float stepRadius = light.Radius * t;

            TEColor stepColor(color.GetValue().r, color.GetValue().g, color.GetValue().b, stepAlpha);
            auto stepMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
            stepMaterial->SetColor(stepColor);
            
            // Re-calculate cone points for this step radius
            TEVector2 sp2 = {position.x + (dir.x * cos(outerRad) - dir.y * sin(outerRad)) * stepRadius,
                            position.y + (dir.x * sin(outerRad) + dir.y * cos(outerRad)) * stepRadius};
            TEVector2 sp3 = {position.x + (dir.x * cos(-outerRad) - dir.y * sin(-outerRad)) * stepRadius,
                            position.y + (dir.x * sin(-outerRad) + dir.y * cos(-outerRad)) * stepRadius};
            SubmitTriangle(p1, sp2, sp3, stepMaterial);
        }
    }
    else if (light.Type == TELightType::Line) {
        // Line Light visualization (a quad of Width)
        TEVector2 offset = light.LineOffset;
        TEVector2 p1 = position;
        TEVector2 p2 = {position.x + offset.x, position.y + offset.y};
        
        // Draw as a simple line (center + offset)
        SubmitQuad(position, {light.Width, offset.Length()}, debugMaterial);
    }
}

void Renderer2D::SubmitRectOutline(const TEVector2& position, const TEVector2& size, float thickness, const TEColor& color) {
    auto debugMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
    debugMaterial->SetColor(color);

    // position is center, size is total width/height
    // Top
    SubmitQuad({position.x, position.y + size.y * 0.5f}, {size.x, thickness}, debugMaterial);
    // Bottom
    SubmitQuad({position.x, position.y - size.y * 0.5f}, {size.x, thickness}, debugMaterial);
    // Left
    SubmitQuad({position.x - size.x * 0.5f, position.y}, {thickness, size.y}, debugMaterial);
    // Right
    SubmitQuad({position.x + size.x * 0.5f, position.y}, {thickness, size.y}, debugMaterial);
}

void Renderer2D::SubmitCircleOutline(const TEVector2& center, float radius, float thickness, const TEColor& color) {
    auto debugMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
    debugMaterial->SetColor(color);

    // Approximate circle with lines (triangles)
    const int segments = 32;
    for (int i = 0; i < segments; i++) {
        float angle1 = (float)i / segments * 2.0f * 3.14159f;
        float angle2 = (float)(i + 1) / segments * 2.0f * 3.14159f;

        TEVector2 p1 = { center.x + cos(angle1) * radius, center.y + sin(angle1) * radius };
        TEVector2 p2 = { center.x + cos(angle2) * radius, center.y + sin(angle2) * radius };
        
        // Since we don't have a line renderer, we can use a very thin quad or just a thin triangle
        // For debugging, a simple triangle is fine or we can just use SubmitTriangle if we don't mind the fill
        // But the user wants an outline. For now, I'll just use a small quad for each segment.
        TEVector2 mid = (p1 + p2) * 0.5f;
        TEVector2 dir = p2 - p1;
        SubmitQuad(p1, {dir.Length(), thickness}, debugMaterial); // This needs rotation!
    }
}

std::shared_ptr<Renderer2D> Renderer2D::Create() {
    return std::make_shared<Renderer2D>();
}

} // namespace TE