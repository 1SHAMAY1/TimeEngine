#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utility/MathUtils.hpp"

namespace TE
{

class BoxComponent : public ProceduralSpriteComponent
{
public:
    TEVector2 Size = {1.0f, 1.0f};

    // Collision Data
    bool bHasCollision = true;
    bool bShowDebug = false;
    float Density = 1.0f;
    float Friction = 0.5f;

    virtual const char *GetClassName() const override { return StaticClassName; }

    std::vector<TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const override
    {
        float hx = Size.x * 0.5f, hy = Size.y * 0.5f;
        glm::vec4 local[4] = {{-hx,-hy,0,1},{hx,-hy,0,1},{hx,hy,0,1},{-hx,hy,0,1}};
        std::vector<TEVector2> v;
        for (int i = 0; i < 4; i++) { glm::vec4 w = worldModel * local[i]; v.push_back({w.x, w.y}); }
        return v;
    }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        glm::vec2 pos = {worldModel[3].x, worldModel[3].y};
        glm::vec2 sz = {Size.x * glm::length(glm::vec3(worldModel[0])), Size.y * glm::length(glm::vec3(worldModel[1]))};
        return point.x >= pos.x - sz.x*0.5f && point.x <= pos.x + sz.x*0.5f &&
               point.y >= pos.y - sz.y*0.5f && point.y <= pos.y + sz.y*0.5f;
    }

    bool CastsOcclusionShadow() const override { return bHasCollision && bIsVisible; }

    void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel, const std::shared_ptr<class TE::Material> &material) const override
    {
        if (bIsVisible)
        {
            material->SetColor(BaseColor);
            renderer->SubmitQuad(glm::scale(worldModel, glm::vec3(Size.x, Size.y, 1.0f)), material);
        }
        if (bShowDebug)
        {
            renderer->SubmitRectOutline(TE::TEVector2(worldModel[3].x, worldModel[3].y),
                                        {Size.x * glm::length(glm::vec3(worldModel[0])), Size.y * glm::length(glm::vec3(worldModel[1]))}, 0.05f,
                                        TE::TEColor(0.2f, 1.0f, 0.2f, 1.0f));
        }
    }

    static constexpr const char *StaticClassName = "BoxComponent";
};

} // namespace TE
