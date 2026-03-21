#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class TriangleComponent : public ProceduralSpriteComponent
{
public:
    TEVector2 Point1 = {-0.5f, -0.5f};
    TEVector2 Point2 = {0.5f, -0.5f};
    TEVector2 Point3 = {0.0f, 0.5f};

    // Collision Data
    bool bHasCollision = true;
    bool bShowDebug = false;
    float Density = 1.0f;
    float Friction = 0.5f;

    virtual const char *GetClassName() const override { return StaticClassName; }

    std::vector<TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const override
    {
        std::vector<TEVector2> v;
        glm::vec4 w1 = worldModel * glm::vec4(Point1.x, Point1.y, 0, 1);
        glm::vec4 w2 = worldModel * glm::vec4(Point2.x, Point2.y, 0, 1);
        glm::vec4 w3 = worldModel * glm::vec4(Point3.x, Point3.y, 0, 1);
        v.push_back({w1.x, w1.y}); v.push_back({w2.x, w2.y}); v.push_back({w3.x, w3.y});
        return v;
    }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        glm::vec4 w1 = worldModel * glm::vec4(Point1.x, Point1.y, 0, 1);
        glm::vec4 w2 = worldModel * glm::vec4(Point2.x, Point2.y, 0, 1);
        glm::vec4 w3 = worldModel * glm::vec4(Point3.x, Point3.y, 0, 1);
        float minX = std::min({w1.x, w2.x, w3.x}), maxX = std::max({w1.x, w2.x, w3.x});
        float minY = std::min({w1.y, w2.y, w3.y}), maxY = std::max({w1.y, w2.y, w3.y});
        return point.x >= minX && point.x <= maxX && point.y >= minY && point.y <= maxY;
    }

    bool CastsOcclusionShadow() const override { return bHasCollision && bIsVisible; }

    void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel, const std::shared_ptr<class TE::Material> &material) const override
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
        if (bShowDebug)
        {
            renderer->SubmitTriangle(p1, p2, p3, material); // Just simple debug fill for now
        }
    }

    static constexpr const char *StaticClassName = "TriangleComponent";
};

} // namespace TE
