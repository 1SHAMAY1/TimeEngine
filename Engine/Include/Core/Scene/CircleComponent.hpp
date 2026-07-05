#pragma once
#include "Core/Collision/CircleColliderComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utils/MathUtils.hpp"

namespace TE
{

class CircleComponent : public ProceduralSpriteComponent
{
public:
    GENERATED_BODY(CircleComponent)

    T_PROPERTY(float, Radius, "Radius", 0.5f)
    T_PROPERTY(TEColor, BaseColor, "Base Color", TEColor::White())
    T_PROPERTY(bool, bIsVisible, "Visible", true)

    virtual void OnInitialize() override
    {
        ProceduralSpriteComponent::OnInitialize();
        auto *collider = GetOwnerEntity().AddComponent<CircleColliderComponent>();
        collider->Radius = Radius;
    }

    virtual const char *GetClassName() const override { return StaticClassName; }

    std::vector<TEVector2> GetWorldVertices(const TE::TEMatrix4 &worldModel) const override
    {
        std::vector<TEVector2> v;
        for (int i = 0; i < 12; i++)
        {
            float angle = (float)i / 12.0f * 2.0f * 3.14159265f;
            TEVector4 p = worldModel * TEVector4(cos(angle) * Radius, sin(angle) * Radius, 0, 1);
            v.push_back({p.x, p.y});
        }
        return v;
    }

    bool ContainsPoint(const TE::TEMatrix4 &worldModel, const TEVector2 &point) const override
    {
        auto *collider = GetOwnerEntity().GetComponent<CircleColliderComponent>();
        if (collider)
        {
            float dx = point.x - collider->shape.circle.center.x;
            float dy = point.y - collider->shape.circle.center.y;
            return (dx * dx + dy * dy) <= collider->shape.circle.radius * collider->shape.circle.radius;
        }
        return false;
    }

    void OnRender(class TE::Renderer2D *renderer, const TE::TEMatrix4 &worldModel,
                  const std::shared_ptr<class TE::Material> &material) const override
    {
        TE::TEVector2 worldPos = {worldModel.m[3][0], worldModel.m[3][1]};
        float radius =
            Radius * std::sqrt(worldModel.m[0][0] * worldModel.m[0][0] + worldModel.m[0][1] * worldModel.m[0][1] +
                               worldModel.m[0][2] * worldModel.m[0][2]);

        if (bIsVisible)
        {
            material->SetColor(BaseColor);
            renderer->SubmitCircle(worldPos, radius, material);
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CircleComponent, "Circle Component")
T_REGISTER_PROPERTY(CircleComponent, float, Radius, "Radius")
T_REGISTER_PROPERTY(CircleComponent, TEColor, BaseColor, "Base Color")
T_REGISTER_PROPERTY(CircleComponent, bool, bIsVisible, "Visible")
T_REGISTER_PRESET(Circle, "Circle", "Shapes",
                  ([](::TE::EntityID id, ::TE::EntityManager *em) { em->AddComponent<CircleComponent>(id); }))
#endif

} // namespace TE
