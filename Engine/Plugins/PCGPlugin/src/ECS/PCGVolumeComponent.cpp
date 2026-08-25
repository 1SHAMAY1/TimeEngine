#include "ECS/PCGVolumeComponent.hpp"
#include "Asset/PCGGraphSerializer.hpp"
#include "Core/Log.h"
#include "Core/Scene/Scene.hpp"

void PCGVolumeComponent::OnInitialize()
{
    TComponent::OnInitialize();
    m_HasGenerated = false;

    if (!GraphAssetPath.empty())
    {
        PCGGraphSerializer::LoadFromFile(m_RuntimeGraph, GraphAssetPath);
    }
}

void PCGVolumeComponent::Tick(float deltaTime)
{
    if (AutoGenerateOnPlay && !m_HasGenerated)
    {
        Generate();
    }
}

bool PCGVolumeComponent::Generate(Ref<Scene> scene)
{
    if (GraphAssetPath.empty())
    {
        TE_CORE_WARN("[PCGVolumeComponent] No PCG Graph Asset assigned.");
        return false;
    }

    if (m_RuntimeGraph.GetNodes().IsEmpty())
    {
        PCGGraphSerializer::LoadFromFile(m_RuntimeGraph, GraphAssetPath);
    }

    float halfW = BoundsWidth * 0.5f;
    float halfH = BoundsHeight * 0.5f;

    PCGExecutionContext ctx(
        static_cast<uint32_t>(Seed),
        glm::vec3(-halfW, -halfH, 0.0f),
        glm::vec3(halfW, halfH, 0.0f),
        scene ? scene.get() : nullptr
    );

    bool ok = m_RuntimeGraph.Execute(ctx);
    m_HasGenerated = ok;
    return ok;
}

void PCGVolumeComponent::ClearGenerated()
{
    m_HasGenerated = false;
}
