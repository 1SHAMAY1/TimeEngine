#pragma once
#include "Core/Collision/TriangleColliderComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

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

    virtual const char *GetClassName() const override { return StaticClassName; }

    std::vector<TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const override
    {
        std::vector<TEVector2> v;
        glm::vec4 w1 = worldModel * glm::vec4(Point1.x, Point1.y, 0, 1);
        glm::vec4 w2 = worldModel * glm::vec4(Point2.x, Point2.y, 0, 1);
        glm::vec4 w3 = worldModel * glm::vec4(Point3.x, Point3.y, 0, 1);
        v.push_back({w1.x, w1.y});
        v.push_back({w2.x, w2.y});
        v.push_back({w3.x, w3.y});
        return v;
    }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        auto *collider = GetOwnerEntity().GetComponent<TriangleColliderComponent>();
        if (collider)
        {
            const auto *pts = &collider->Vertices0;
            float minX = std::min({pts[0].x, pts[1].x, pts[2].x});
            float maxX = std::max({pts[0].x, pts[1].x, pts[2].x});
            float minY = std::min({pts[0].y, pts[1].y, pts[2].y});
            float maxY = std::max({pts[0].y, pts[1].y, pts[2].y});
            return point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY;
        }
        return false;
    }

    void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel,
                  const std::shared_ptr<class TE::Material> &material) const override
    {
        auto TransformPoint = [&](const TEVector2 &p)
        {
            glm::vec4 tp = worldModel * glm::vec4(p.x, p.y, 0.0f, 1.0f);
            return TE::TEVector2(tp.x, tp.y);
        };

        TE::TEVector2 p1 = TransformPoint(Point1);
        TE::TEVector2 p2 = TransformPoint(Point2);
        TE::TEVector2 p3 = TransformPoint(Point3);

        if (bIsVisible)
        {
            material->SetColor(BaseColor);
            renderer->SubmitTriangle(p1, p2, p3, material);
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
                  ([](::TE::EntityID id, ::TE::EntityManager *em) { em->AddComponent<TriangleComponent>(id); }))
#endif

} // namespace TE
