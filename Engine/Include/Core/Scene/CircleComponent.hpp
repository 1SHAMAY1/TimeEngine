#pragma once
#include "Core/Collision/CircleColliderComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utils/MathUtils.hpp"


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

    virtual TEString GetClassName() const override { return StaticClassName; }

    TEArray<TEVector2> GetWorldVertices(const TEMatrix4 &worldModel) const override
    {
        TEArray<TEVector2> v;
        for (int i = 0; i < 12; i++)
        {
            float angle = (float)i / 12.0f * 2.0f * 3.14159265f;
            TEVector4 p = worldModel * TEVector4(cos(angle) * Radius, sin(angle) * Radius, 0, 1);
            v.Add({p.x, p.y});
        }
        return v;
    }

    bool ContainsPoint(const TEMatrix4 &worldModel, const TEVector2 &point) const override
    {
        TEVector2 worldCenter(worldModel[3][0], worldModel[3][1]);
        float scaleX = TEVector3(worldModel[0][0], worldModel[0][1], worldModel[0][2]).Length();
        float worldRadius = Radius * (scaleX > 0.0001f ? scaleX : 1.0f);
        float dx = point.x - worldCenter.x;
        float dy = point.y - worldCenter.y;
        return (dx * dx + dy * dy) <= (worldRadius * worldRadius);
    }

    void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldModel,
                  const TERef<class Material> &material) const override
    {
        if (!bIsVisible || !renderer)
            return;

        TERef<Material> renderMat = material ? material : Material::GetDefault();
        if (renderMat)
        {
            renderMat->SetColor(BaseColor);

            TEVector2 worldPos = {worldModel.m[3][0], worldModel.m[3][1]};
            float scaleX = worldModel.m[0][0], scaleY = worldModel.m[0][1], scaleZ = worldModel.m[0][2];
            float radius = Radius * Sqrt(scaleX * scaleX + scaleY * scaleY + scaleZ * scaleZ);
            renderer->SubmitCircle(worldPos, radius, renderMat);
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(CircleComponent, "Circle Component")
T_REGISTER_PROPERTY(CircleComponent, float, Radius, "Radius")
T_REGISTER_PROPERTY(CircleComponent, TEColor, BaseColor, "Base Color")
T_REGISTER_PROPERTY(CircleComponent, bool, bIsVisible, "Visible")
T_REGISTER_PRESET(Circle, "Circle", "Shapes",
                  ([](EntityID id, EntityManager *em) { em->AddComponent<CircleComponent>(id); }))
#endif

