#include "Core/PreRequisites.h"
#include "Core/Scene/NavigationAgent2DComponent.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/Scene.hpp"
#include "Core/Log.h"

void NavigationAgent2DComponent::SetTarget(const TEVector2 &worldPos)
{
    TargetPosition = worldPos;
    m_TargetEntityID = 0;
    m_bHasReachedTarget = false;
    m_bIsNavigating = true;
    m_PathTimer = 0.0f;
    RecalculatePath();
}

void NavigationAgent2DComponent::SetTargetEntity(uint64_t targetEntityID)
{
    m_TargetEntityID = targetEntityID;
    m_bHasReachedTarget = false;
    m_bIsNavigating = true;
    m_PathTimer = 0.0f;
    RecalculatePath();
}

void NavigationAgent2DComponent::StopNavigation()
{
    m_bIsNavigating = false;
    m_Path.Clear();
    m_WaypointIndex = 0;
    Velocity = {0.0f, 0.0f};
}

void NavigationAgent2DComponent::RecalculatePath()
{
    if (!Manager)
        return;

    TEVector2 currentPos = {Transform.Position.x, Transform.Position.y};
    TEVector2 targetPos = TargetPosition;

    if (m_TargetEntityID != 0)
    {
        Entity targetEnt(m_TargetEntityID, Manager);
        if (targetEnt.IsValid())
        {
            const auto &t = targetEnt.GetTransform();
            targetPos = {t.Position.x, t.Position.y};
            TargetPosition = targetPos;
        }
    }

    float distToTarget = (targetPos - currentPos).Length();
    if (distToTarget <= StoppingDistance)
    {
        m_bHasReachedTarget = true;
        m_bIsNavigating = false;
        m_Path.Clear();
        Velocity = {0.0f, 0.0f};
        return;
    }

    m_Path = NavigationMesh2D::Get().FindPath2D(currentPos, targetPos);
    m_WaypointIndex = 0;

    // Skip the first waypoint if we are already close to it
    if (!m_Path.empty())
    {
        if ((m_Path[0] - currentPos).Length() < StoppingDistance && m_Path.size() > 1)
        {
            m_WaypointIndex = 1;
        }
    }
}

void NavigationAgent2DComponent::UpdateMovement(float dt)
{
    if (!bMovementEnabled || !m_bIsNavigating)
    {
        Velocity = {0.0f, 0.0f};
        return;
    }

    m_PathTimer += dt;
    if (m_PathTimer >= PathUpdateInterval)
    {
        m_PathTimer = 0.0f;
        RecalculatePath();
    }

    if (m_Path.empty() || m_WaypointIndex >= (int)m_Path.size())
    {
        m_bHasReachedTarget = true;
        m_bIsNavigating = false;
        Velocity = {0.0f, 0.0f};
        return;
    }

    TEVector2 currentPos = {Transform.Position.x, Transform.Position.y};
    TEVector2 targetWaypoint = m_Path[m_WaypointIndex];
    TEVector2 toWaypoint = targetWaypoint - currentPos;
    float dist = toWaypoint.Length();

    if (dist <= StoppingDistance)
    {
        m_WaypointIndex++;
        if (m_WaypointIndex >= (int)m_Path.size())
        {
            m_bHasReachedTarget = true;
            m_bIsNavigating = false;
            Velocity = {0.0f, 0.0f};
            return;
        }
        targetWaypoint = m_Path[m_WaypointIndex];
        toWaypoint = targetWaypoint - currentPos;
        dist = toWaypoint.Length();
    }

    if (dist > 0.001f)
    {
        TEVector2 moveDir = toWaypoint / dist;
        Velocity = moveDir * MaxSpeed;
        Transform.Position.x += Velocity.x * dt;
        Transform.Position.y += Velocity.y * dt;
    }
    else
    {
        Velocity = {0.0f, 0.0f};
    }
}
