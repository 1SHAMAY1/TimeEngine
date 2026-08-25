#pragma once
#include "Core/Collision/TriangleColliderComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utils/MathUtils.hpp"


class TriangleComponent : public ProceduralSpriteComponent
{
public:
    GENERATED_BODY(TriangleComponent)

    T_PROPERTY(TEVector2, Point1, "Point 1", TEVector2(-0.5f, -0.5f))
    T_PROPERTY(TEVector2, Point2, "Point 2", TEVector2(0.5f, -0.5f))
    T_PROPERTY(TEVector2, Point3, "Point 3", TEVector2(0.0f, 0.5f))
    T_PROPERTY(TEColor, BaseColor, "Base Color", TEColor::White())
    T_PROPERTY(bool, bIsVisible, "Visible", true)

    virtual void OnInitialize() override
    {
        ProceduralSpriteComponent::OnInitialize();
        auto *collider = GetOwnerEntity().AddComponent<TriangleColliderComponent>();
        collider->Vertices0 = Point1;
        collider->Vertices1 = Point2;
        collider->Vertices2 = Point3;
    }

    virtual TEString GetClassName() const override { return StaticClassName; }

    TEArray<TEVector2> GetWorldVertices(const TEMatrix4 &worldModel) const override
    {
        TEArray<TEVector2> v;
        TEVector4 w1 = worldModel * TEVector4(Point1.x, Point1.y, 0, 1);
        TEVector4 w2 = worldModel * TEVector4(Point2.x, Point2.y, 0, 1);
        TEVector4 w3 = worldModel * TEVector4(Point3.x, Point3.y, 0, 1);
        v.Add({w1.x, w1.y});
        v.Add({w2.x, w2.y});
        v.Add({w3.x, w3.y});
        return v;
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

            TEVector4 w1 = worldModel * TEVector4(Point1.x, Point1.y, 0.0f, 1.0f);
            TEVector4 w2 = worldModel * TEVector4(Point2.x, Point2.y, 0.0f, 1.0f);
            TEVector4 w3 = worldModel * TEVector4(Point3.x, Point3.y, 0.0f, 1.0f);

            renderer->SubmitTriangle(TEVector2(w1.x, w1.y), TEVector2(w2.x, w2.y), TEVector2(w3.x, w3.y), renderMat);
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(TriangleComponent, "Triangle Component")
T_REGISTER_PROPERTY(TriangleComponent, TEVector2, Point1, "Point 1")
T_REGISTER_PROPERTY(TriangleComponent, TEVector2, Point2, "Point 2")
T_REGISTER_PROPERTY(TriangleComponent, TEVector2, Point3, "Point 3")
T_REGISTER_PROPERTY(TriangleComponent, TEColor, BaseColor, "Base Color")
T_REGISTER_PROPERTY(TriangleComponent, bool, bIsVisible, "Visible")
T_REGISTER_PRESET(Triangle, "Triangle", "Shapes",
                  ([](EntityID id, EntityManager *em) { em->AddComponent<TriangleComponent>(id); }))
#endif

