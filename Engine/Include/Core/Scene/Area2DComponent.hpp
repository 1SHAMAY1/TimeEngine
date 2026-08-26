#pragma once

#include "Core/Collision/CollisionComponent.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include <functional>
#include <unordered_set>

class TE_API Area2DComponent : public CollisionComponent
{
public:
    GENERATED_BODY(Area2DComponent)
    T_EVENT_VISIBLE(TScriptEventType::CollisionEvent | TScriptEventType::AreaEvent)

    T_PROPERTY(TEVector2, Size, "Size", TEVector2(100.0f, 100.0f))
    T_PROPERTY(bool, MonitoringEnabled, "Monitoring Enabled", true)

    std::function<void(EntityID)> OnAreaEntered;
    std::function<void(EntityID)> OnAreaExited;

    Area2DComponent();
    virtual ~Area2DComponent() override = default;

    virtual void OnAttach() override;
    virtual void Tick(float deltaTime) override;
    virtual void OnUpdateShape(const TEMatrix4 &worldTransform) override;

    void UpdateOverlaps(const TEArray<EntityID> &newOverlaps);
    const TESet<EntityID> &GetOverlappingEntities() const { return m_CurrentOverlaps; }
    bool HasOverlap(EntityID id) const { return m_CurrentOverlaps.Contains(id); }

    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TESet<EntityID> m_CurrentOverlaps;
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(Area2DComponent, "Area 2D Component")
T_REGISTER_PROPERTY(Area2DComponent, TEVector2, Size, "Size")
T_REGISTER_PROPERTY(Area2DComponent, bool, MonitoringEnabled, "Monitoring Enabled")
T_REGISTER_PRESET(Area2DComponent, "Area 2D (Trigger)", "Physics & Collisions",
                  [](EntityID id, EntityManager *em) { em->AddComponent<Area2DComponent>(id); })
#endif
