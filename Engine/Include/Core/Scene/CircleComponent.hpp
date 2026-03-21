#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class CircleComponent : public ProceduralSpriteComponent
{
public:
    float Radius = 0.5f;

    // Collision Data
    bool bHasCollision = true;
    bool bShowDebug = false;
    float Density = 1.0f;
    float Friction = 0.5f;

    virtual const char *GetClassName() const override { return StaticClassName; }

    std::vector<TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const override
    {
        std::vector<TEVector2> v;
        for (int i = 0; i < 12; i++)
        {
            float angle = (float)i / 12.0f * 2.0f * 3.14159265f;
            glm::vec4 p = worldModel * glm::vec4(cos(angle) * Radius, sin(angle) * Radius, 0, 1);
            v.push_back({p.x, p.y});
        }
        return v;
    }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        glm::vec2 pos = {worldModel[3].x, worldModel[3].y};
        float r = Radius * glm::length(glm::vec3(worldModel[0]));
        float dx = point.x - pos.x, dy = point.y - pos.y;
        return (dx * dx + dy * dy) <= r * r;
    }

    bool CastsOcclusionShadow() const override { return bHasCollision && bIsVisible; }

    void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel,
                  const std::shared_ptr<class TE::Material> &material) const override
    {
        TE::TEVector2 worldPos = {worldModel[3].x, worldModel[3].y};
        float radius = Radius * glm::length(glm::vec3(worldModel[0]));

        if (bIsVisible)
        {
            material->SetColor(BaseColor);
            renderer->SubmitCircle(worldPos, radius, material);
        }
        if (bShowDebug)
        {
            renderer->SubmitCircleOutline(worldPos, radius, 0.05f, TE::TEColor(0.2f, 1.0f, 0.2f, 1.0f));
        }
    }

    static constexpr const char *StaticClassName = "CircleComponent";
};

} // namespace TE
