#pragma once
#include "Renderer/RenderBatcher.hpp"
#include "Renderer/Renderer.hpp"
#include "Utils/MathUtils.hpp"

class LightComponent;

class TE_API Renderer2D : public Renderer
{
public:
    static TERef<Renderer2D> Create();
    Renderer2D();
    virtual ~Renderer2D() override;

    void BeginFrame(const TEMatrix4 &viewProjection) override;
    void Submit(const TERef<VertexArray> &vao, const TERef<Material> &material,
                const TEMatrix4 &transform, uint32_t indexCount) override;
    void EndFrame() override;
    void Flush() override;

    // 2D-specific API (example: submit a quad)
    void SubmitQuad(const TEVector2 &position, const TEVector2 &size, const TERef<Material> &material);
    void SubmitQuad(const TEMatrix4 &transform, const TERef<Material> &material, int blendMode = 0);
    void SubmitTriangle(const TEVector2 &p1, const TEVector2 &p2, const TEVector2 &p3,
                        const TERef<Material> &material);
    void SubmitCircle(const TEVector2 &center, float radius, const TERef<Material> &material);
    void SubmitLine(const TEVector2 &p1, const TEVector2 &p2, float thickness, const TEColor &color);
    void SubmitLight(const class LightComponent &light, const TEVector2 &position, float rotation = 0.0f);
    void SubmitShadow(const TEVector2 &lightPos, float lightRadius, const TEArray<TEVector2> &vertices);
    void SetAmbientLight(const TEColor &color, float intensity);
    void SetAmbientGradient(const TEColor &sky, const TEColor &horizon, const TEColor &ground, float intensity = 1.0f,
                            float horizonHeight = 0.5f, float horizonSpread = 0.2f);

    // Debug Outlines
    void SubmitRectOutline(const TEVector2 &position, const TEVector2 &size, float thickness, const TEColor &color);
    void SubmitCircleOutline(const TEVector2 &center, float radius, float thickness, const TEColor &color);

private:
    RenderBatcher m_Batcher;

    TERef<VertexArray> m_UnitQuadVAO;
    TERef<Material> m_Light2DMaterial;

    // Ambient Gradient State (No default ambient light)
    TEColor m_AmbientSky = TEColor(0.04f, 0.04f, 0.06f, 1.0f);
    TEColor m_AmbientHorizon = TEColor(0.08f, 0.08f, 0.12f, 1.0f);
    TEColor m_AmbientGround = TEColor(0.02f, 0.02f, 0.02f, 1.0f);
    float m_AmbientIntensity = 0.0f;
    float m_AmbientHorizonHeight = 0.5f;
    float m_AmbientHorizonSpread = 0.2f;
};

