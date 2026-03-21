#include "Renderer/Renderer2D.hpp"
#include "Core/Scene/LightComponent.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/ShaderLibrary.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"
#include "Utility/MathUtils.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace TE
{

Renderer2D::Renderer2D()
{
    m_Light2DMaterial = std::make_shared<Material>(ShaderLibrary::CreateLight2DShader());

    // Initialize Unit Quad VAO for optimization
    float quadVertices[] = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f};
    uint32_t quadIndices[] = {0, 1, 2, 2, 3, 0};

    m_UnitQuadVAO = std::shared_ptr<VertexArray>(VertexArray::Create());
    auto vbo = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
    auto ibo = IndexBuffer::Create(quadIndices, 6);
    m_UnitQuadVAO->Bind();
    vbo->Bind();
    m_UnitQuadVAO->AddVertexBuffer(vbo);
    m_UnitQuadVAO->SetIndexBuffer(ibo);
}
Renderer2D::~Renderer2D() {}

void Renderer2D::BeginFrame(const glm::mat4 &viewProjection)
{
    m_Batcher.SetViewProjection(viewProjection);
    m_Batcher.Begin();
}

void Renderer2D::Submit(const std::shared_ptr<VertexArray> &vao, const std::shared_ptr<Material> &material,
                        const glm::mat4 &transform, uint32_t indexCount)
{
    m_Batcher.Submit(vao, material, transform, indexCount);
}

void Renderer2D::EndFrame()
{
    // Draw Ambient Light (Fullscreen additive quad)
    if (m_AmbientIntensity > 0.0f)
    {
        static std::shared_ptr<Material> ambientMaterial = nullptr;
        if (!ambientMaterial)
            ambientMaterial = std::make_shared<Material>(ShaderLibrary::CreateAmbientGradientShader());

        ambientMaterial->SetUniform("u_SkyColor", m_AmbientSky);
        ambientMaterial->SetUniform("u_HorizonColor", m_AmbientHorizon);
        ambientMaterial->SetUniform("u_GroundColor", m_AmbientGround);
        ambientMaterial->SetUniform("u_Intensity", m_AmbientIntensity);
        ambientMaterial->SetUniform("u_HorizonHeight", m_AmbientHorizonHeight);
        ambientMaterial->SetUniform("u_HorizonSpread", m_AmbientHorizonSpread);

        // Submit very large quad to cover entire view (Ambient Light)
        SubmitQuad(glm::scale(glm::mat4(1.0f), glm::vec3(1000000.0f, 1000000.0f, 1.0f)), ambientMaterial, true);
    }

    m_Batcher.End();
    m_Batcher.Flush();
}

void Renderer2D::Flush() { m_Batcher.Flush(); }

void Renderer2D::SubmitQuad(const TEVector2 &position, const TEVector2 &size, const std::shared_ptr<Material> &material)
{
    TETransform transform;
    transform.Position = glm::vec3(position.x, position.y, 0.0f);
    transform.Scale = TEScale(size.x, size.y, 1.0f);
    SubmitQuad(transform.GetMatrix(), material);
}

void Renderer2D::SubmitQuad(const glm::mat4 &transform, const std::shared_ptr<Material> &material, int blendMode)
{
    if (blendMode == 0)
    {
        material->SetUniform("u_AmbientIntensity", m_AmbientIntensity);
        material->SetUniform("u_AmbientSky", m_AmbientSky);
        material->SetUniform("u_AmbientGround", m_AmbientGround);
    }
    m_Batcher.Submit(m_UnitQuadVAO, material, transform, 6, blendMode);
}

void Renderer2D::SubmitTriangle(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                                const std::shared_ptr<Material> &material)
{
    float vertices[] = {p1.x, p1.y, 0.0f, p2.x, p2.y, 0.0f, p3.x, p3.y, 0.0f};
    uint32_t indices[] = {0, 1, 2};
    auto vao = std::shared_ptr<VertexArray>(VertexArray::Create());
    auto vbo = VertexBuffer::Create(vertices, sizeof(vertices));
    auto ibo = IndexBuffer::Create(indices, 3);
    vao->Bind();
    vbo->Bind();
    vao->AddVertexBuffer(vbo);
    vao->SetIndexBuffer(ibo);
    material->SetUniform("u_AmbientIntensity", m_AmbientIntensity);
    material->SetUniform("u_AmbientSky", m_AmbientSky);
    material->SetUniform("u_AmbientGround", m_AmbientGround);
    m_Batcher.Submit(vao, material, glm::mat4(1.0f), 3);
}

void Renderer2D::SubmitCircle(const TEVector2 &center, float radius, const std::shared_ptr<Material> &material)
{
    const int segments = 32;
    std::vector<float> vertices;
    vertices.push_back(center.x);
    vertices.push_back(center.y);
    vertices.push_back(0.0f);
    for (int i = 0; i <= segments; ++i)
    {
        float angle = 2.0f * 3.14159265f * i / segments;
        vertices.push_back(center.x + std::cos(angle) * radius);
        vertices.push_back(center.y + std::sin(angle) * radius);
        vertices.push_back(0.0f);
    }
    std::vector<uint32_t> indices;
    for (int i = 1; i <= segments; ++i)
    {
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
    material->SetUniform("u_AmbientIntensity", m_AmbientIntensity);
    material->SetUniform("u_AmbientSky", m_AmbientSky);
    material->SetUniform("u_AmbientGround", m_AmbientGround);
    m_Batcher.Submit(vao, material, glm::mat4(1.0f), (uint32_t)indices.size());
}

void Renderer2D::SubmitLight(const LightComponent &light, const TEVector2 &position, float rotationRadians)
{
    static std::shared_ptr<Shader> lightShader = nullptr;
    if (!lightShader)
        lightShader = ShaderLibrary::CreateLight2DShader();

    auto lightMaterial = std::make_shared<Material>(lightShader);

    lightMaterial->SetColor(light.Color);
    lightMaterial->SetUniform("u_Intensity", light.Intensity);
    lightMaterial->SetUniform("u_FalloffExponent", light.FalloffExponent);
    lightMaterial->SetUniform("u_LightType", (int)light.Type);

    if (light.Type == TELightType::Point)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(light.Radius * 2.0f, light.Radius * 2.0f, 1.0f));
        SubmitQuad(transform, lightMaterial, 1);
    }
    else if (light.Type == TELightType::Spot)
    {
        float baseAngle = atan2(light.Direction.y, light.Direction.x);
        float finalAngle = baseAngle + rotationRadians;
        glm::vec2 finalDirection = {cos(finalAngle), sin(finalAngle)};

        lightMaterial->SetUniform("u_Direction", finalDirection);
        lightMaterial->SetUniform("u_InnerAngle", light.InnerAngle);
        lightMaterial->SetUniform("u_OuterAngle", light.OuterAngle);

        glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.0f)) *
                              glm::rotate(glm::mat4(1.0f), rotationRadians, glm::vec3(0.0f, 0.0f, 1.0f)) *
                              glm::scale(glm::mat4(1.0f), glm::vec3(light.Radius * 2.0f, light.Radius * 2.0f, 1.0f));
        SubmitQuad(transform, lightMaterial, 1);
    }
    else if (light.Type == TELightType::Line)
    {
        float length = sqrt(light.LineOffset.x * light.LineOffset.x + light.LineOffset.y * light.LineOffset.y);
        float angle = atan2(light.LineOffset.y, light.LineOffset.x);

        lightMaterial->SetUniform("u_LineLength", length);
        lightMaterial->SetUniform("u_Radius", light.Radius);

        // Translate to middle of line
        glm::mat4 transform =
            glm::translate(glm::mat4(1.0f), glm::vec3(position.x + light.LineOffset.x * 0.5f,
                                                      position.y + light.LineOffset.y * 0.5f, 0.0f)) *
            glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(length + light.Radius * 2.0f, light.Radius * 2.0f, 1.0f));
        SubmitQuad(transform, lightMaterial, 1);
    }
}

void Renderer2D::SubmitShadow(const TEVector2 &lightPos, float lightRadius, const std::vector<TEVector2> &vertices)
{
    if (vertices.size() < 2)
        return;

    // Find the two silhouette vertices: the ones that form the widest angle from the light.
    glm::vec2 lp(lightPos.x, lightPos.y);

    // Compute angles from light to each vertex
    std::vector<float> angles;
    angles.reserve(vertices.size());
    for (const auto &v : vertices)
    {
        glm::vec2 dir = glm::vec2(v.x, v.y) - lp;
        angles.push_back(atan2(dir.y, dir.x));
    }

    // Find the two vertices that form the widest angular spread from the light.
    int bestA = 0, bestB = 1;
    float maxSpread = -1.0f;
    for (size_t i = 0; i < vertices.size(); i++)
    {
        for (size_t j = i + 1; j < vertices.size(); j++)
        {
            float diff = angles[j] - angles[i];
            while (diff > 3.14159265f)
                diff -= 2.0f * 3.14159265f;
            while (diff < -3.14159265f)
                diff += 2.0f * 3.14159265f;
            float absDiff = std::abs(diff);

            if (absDiff > maxSpread)
            {
                maxSpread = absDiff;
                bestA = (int)i;
                bestB = (int)j;
            }
        }
    }

    // Project ray from light through each silhouette vertex
    float projDist = lightRadius * 3.0f;

    glm::vec2 cA(vertices[bestA].x, vertices[bestA].y);
    glm::vec2 cB(vertices[bestB].x, vertices[bestB].y);

    glm::vec2 dirA = glm::normalize(cA - lp);
    glm::vec2 dirB = glm::normalize(cB - lp);

    glm::vec2 farA = cA + dirA * projDist;
    glm::vec2 farB = cB + dirB * projDist;

    static std::shared_ptr<Shader> shadowShader = nullptr;
    if (!shadowShader)
        shadowShader = ShaderLibrary::CreateColorShader();

    auto shadowMat = std::make_shared<Material>(shadowShader);
    shadowMat->SetColor(TEColor(0.0f, 0.0f, 0.0f, 1.0f));

    SubmitTriangle(TEVector2(cA.x, cA.y), TEVector2(cB.x, cB.y), TEVector2(farB.x, farB.y), shadowMat);
    SubmitTriangle(TEVector2(cA.x, cA.y), TEVector2(farB.x, farB.y), TEVector2(farA.x, farA.y), shadowMat);
}

void Renderer2D::SetAmbientLight(const TEColor &color, float intensity)
{
    m_AmbientSky = color;
    m_AmbientHorizon = color;
    m_AmbientGround = color;
    m_AmbientIntensity = intensity;
}

void Renderer2D::SetAmbientGradient(const TEColor &sky, const TEColor &horizon, const TEColor &ground, float intensity,
                                    float horizonHeight, float horizonSpread)
{
    m_AmbientSky = sky;
    m_AmbientHorizon = horizon;
    m_AmbientGround = ground;
    m_AmbientIntensity = intensity;
    m_AmbientHorizonHeight = horizonHeight;
    m_AmbientHorizonSpread = horizonSpread;
}

void Renderer2D::SubmitRectOutline(const TEVector2 &position, const TEVector2 &size, float thickness,
                                   const TEColor &color)
{
    auto debugMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
    debugMaterial->SetColor(color);
    debugMaterial->SetUniform("u_IsUnlit", 1.0f);

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

void Renderer2D::SubmitLine(const TEVector2 &p1, const TEVector2 &p2, float thickness, const TEColor &color)
{
    static std::shared_ptr<Material> lineMaterial = nullptr;
    if (!lineMaterial)
    {
        lineMaterial = std::make_shared<Material>(ShaderLibrary::CreateColorShader());
        lineMaterial->SetUniform("u_IsUnlit", 1.0f);
    }

    lineMaterial->SetColor(color);

    TEVector2 dir = p2 - p1;
    float length = dir.Length();
    if (length < 0.0001f)
        return;

    float angle = atan2(dir.y, dir.x);
    TEVector2 mid = (p1 + p2) * 0.5f;

    glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(mid.x, mid.y, 0.0f)) *
                          glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f)) *
                          glm::scale(glm::mat4(1.0f), glm::vec3(length, thickness, 1.0f));

    SubmitQuad(transform, lineMaterial);
}

void Renderer2D::SubmitCircleOutline(const TEVector2 &center, float radius, float thickness, const TEColor &color)
{
    const int segments = 40; // Reduced from 64 for better performance while keeping it smooth
    for (int i = 0; i < segments; i++)
    {
        float a1 = 2.0f * 3.14159f * (float)i / segments;
        float a2 = 2.0f * 3.14159f * (float)(i + 1) / segments;

        TEVector2 p1 = center + TEVector2(cos(a1) * radius, sin(a1) * radius);
        TEVector2 p2 = center + TEVector2(cos(a2) * radius, sin(a2) * radius);

        SubmitLine(p1, p2, thickness, color);
    }
}

std::shared_ptr<Renderer2D> Renderer2D::Create() { return std::make_shared<Renderer2D>(); }

} // namespace TE