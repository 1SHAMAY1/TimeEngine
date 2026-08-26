#pragma once

#include "Core/Navigation/NavigationMesh2D.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "Core/Scene/MovementComponentBase.hpp"

TE_CLASS()
class TE_API NavigationAgent2DComponent : public MovementComponentBase
{
public:
    TEPROPERTY()
    float StoppingDistance = 8.0f;

    TEPROPERTY()
    float PathUpdateInterval = 0.25f;

    TEPROPERTY()
    bool AvoidanceEnabled = true;

    TEPROPERTY()
    TEVector2 TargetPosition = {0.0f, 0.0f};

    NavigationAgent2DComponent() = default;
    virtual ~NavigationAgent2DComponent() override = default;

    void SetTarget(const TEVector2 &worldPos);
    void SetTargetEntity(uint64_t targetEntityID);
    void StopNavigation();

    bool IsNavigating() const { return m_bIsNavigating; }
    bool HasReachedTarget() const { return m_bHasReachedTarget; }
    const TEArray<TEVector2> &GetCurrentPath() const { return m_Path; }

    virtual void UpdateMovement(float dt) override;

    inline static const TEString StaticClassName = "NavigationAgent2DComponent";
    virtual TEString GetClassName() const override { return StaticClassName; }

private:
    TEArray<TEVector2> m_Path;
    int m_WaypointIndex = 0;
    float m_PathTimer = 0.0f;
    bool m_bIsNavigating = false;
    bool m_bHasReachedTarget = false;
    uint64_t m_TargetEntityID = 0;

    void RecalculatePath();
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(NavigationAgent2DComponent, "Navigation Agent 2D Component")
T_REGISTER_PROPERTY(NavigationAgent2DComponent, float, MaxSpeed, "Max Speed")
T_REGISTER_PROPERTY(NavigationAgent2DComponent, float, StoppingDistance, "Stopping Distance")
T_REGISTER_PROPERTY(NavigationAgent2DComponent, float, PathUpdateInterval, "Path Update Interval")
T_REGISTER_PROPERTY(NavigationAgent2DComponent, bool, AvoidanceEnabled, "Avoidance Enabled")
T_REGISTER_PRESET(NavigationAgent2D, "Navigation Agent 2D", "AI",
                  [](EntityID id, EntityManager *em) { em->AddComponent<NavigationAgent2DComponent>(id); })
#endif
