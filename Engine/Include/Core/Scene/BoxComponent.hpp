#pragma once
#include "Core/Collision/BoxColliderComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utils/MathUtils.hpp"

class BoxComponent : public ProceduralSpriteComponent
{
public:
    GENERATED_BODY(BoxComponent)

    T_PROPERTY(TEVector2, Size, "Size", TEVector2(1.0f, 1.0f))
    T_PROPERTY(TEColor, BaseColor, "Base Color", TEColor::White())
    T_PROPERTY(bool, bIsVisible, "Visible", true)

    virtual void OnInitialize() override
    {
        ProceduralSpriteComponent::OnInitialize();

        auto *collider = GetOwnerEntity().AddComponent<BoxColliderComponent>();
        collider->Size = Size;
    }

    virtual TEString GetClassName() const override { return StaticClassName; }

    TEArray<TEVector2> GetWorldVertices(const TEMatrix4 &worldModel) const override
    {
        float hx = Size.x * 0.5f, hy = Size.y * 0.5f;
        TEVector4 local[4] = {{-hx, -hy, 0, 1}, {hx, -hy, 0, 1}, {hx, hy, 0, 1}, {-hx, hy, 0, 1}};
        TEArray<TEVector2> v;
        for (int i = 0; i < 4; i++)
        {
            TEVector4 w = worldModel * local[i];
            v.Add({w.x, w.y});
        }
        return v;
    }

    void OnRender(class Renderer2D *renderer, const TEMatrix4 &worldModel,
                  const TERef<class Material> &material) const override
    {
        if (!bIsVisible || !renderer)
            return;

        renderer->SubmitQuad(TEMatrix4::Scale(worldModel, TEVector(Size.x, Size.y, 1.0f)), BaseColor, material);
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(BoxComponent, "Box Component")
T_REGISTER_PROPERTY(BoxComponent, TEVector2, Size, "Size")
T_REGISTER_PROPERTY(BoxComponent, TEColor, BaseColor, "Base Color")
T_REGISTER_PROPERTY(BoxComponent, bool, bIsVisible, "Visible")
T_REGISTER_PRESET(Box, "Box", "Shapes", ([](EntityID id, EntityManager *em) { em->AddComponent<BoxComponent>(id); }))
#endif
