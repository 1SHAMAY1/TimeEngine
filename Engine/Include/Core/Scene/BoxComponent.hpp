#pragma once
#include "ProceduralSpriteComponent.hpp"
#include "Renderer/Material.hpp"
#include "Renderer/Renderer2D.hpp"
#include "Utility/MathUtils.hpp"
#include "Core/Collision/BoxColliderComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"

namespace TE
{

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
        
        auto* collider = GetOwnerEntity().AddComponent<BoxColliderComponent>();
        collider->Size = Size;
    }

    virtual const char *GetClassName() const override { return StaticClassName; }

    std::vector<TEVector2> GetWorldVertices(const glm::mat4 &worldModel) const override
    {
        float hx = Size.x * 0.5f, hy = Size.y * 0.5f;
        glm::vec4 local[4] = {{-hx, -hy, 0, 1}, {hx, -hy, 0, 1}, {hx, hy, 0, 1}, {-hx, hy, 0, 1}};
        std::vector<TEVector2> v;
        for (int i = 0; i < 4; i++)
        {
            glm::vec4 w = worldModel * local[i];
            v.push_back({w.x, w.y});
        }
        return v;
    }

    bool ContainsPoint(const glm::mat4 &worldModel, const TEVector2 &point) const override
    {
        auto* collider = GetOwnerEntity().GetComponent<BoxColliderComponent>();
        if (collider) {
            const auto& aabb = collider->shape.aabb;
            return point.x >= aabb.min.x && point.x <= aabb.max.x && 
                   point.y >= aabb.min.y && point.y <= aabb.max.y;
        }
        return false;
    }

    void OnRender(class TE::Renderer2D *renderer, const glm::mat4 &worldModel,
                  const std::shared_ptr<class TE::Material> &material) const override
    {
        if (bIsVisible)
        {
            material->SetColor(BaseColor);
            renderer->SubmitQuad(glm::scale(worldModel, glm::vec3(Size.x, Size.y, 1.0f)), material);
        }
    }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(BoxComponent, "Box Component")
T_REGISTER_PROPERTY(BoxComponent, TEVector2, Size, "Size")
T_REGISTER_PROPERTY(BoxComponent, TEColor, BaseColor, "Base Color")
T_REGISTER_PROPERTY(BoxComponent, bool, bIsVisible, "Visible")
T_REGISTER_PRESET(Box, "Box", "Shapes", ([](::TE::EntityID id, ::TE::EntityManager* em) { em->AddComponent<BoxComponent>(id); }))
#endif

} // namespace TE
