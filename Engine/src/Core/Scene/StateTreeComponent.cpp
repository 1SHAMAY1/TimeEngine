#include "Core/PreRequisites.h"
#include "Core/Scene/StateTreeComponent.hpp"
#include "Core/Asset/AssetManager.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/Scene.hpp"

void StateTreeComponent::OnAttach()
{
    if (StateTreeAssetHandle != 0)
    {
        m_Asset = AssetManager::GetAsset<StateTreeAsset>(StateTreeAssetHandle);
        if (m_Asset && m_Asset->GetGraph())
        {
            Entity agent = GetOwnerEntity();
            m_Evaluator.Initialize(m_Asset->GetGraph(), agent);
        }
    }
}

void StateTreeComponent::SetStateTree(TERef<StateTreeGraph> graph)
{
    Entity agent = GetOwnerEntity();
    m_Evaluator.Initialize(graph, agent);
}

void StateTreeComponent::Tick(float deltaTime)
{
    if (!m_Evaluator.GetGraph() && StateTreeAssetHandle != 0)
    {
        m_Asset = AssetManager::GetAsset<StateTreeAsset>(StateTreeAssetHandle);
        if (m_Asset && m_Asset->GetGraph())
        {
            Entity agent = GetOwnerEntity();
            m_Evaluator.Initialize(m_Asset->GetGraph(), agent);
        }
    }

    m_Evaluator.Tick(deltaTime);
}

const TEString &StateTreeComponent::GetActiveStateName() const
{
    static TEString s_None = "None";
    if (m_Evaluator.GetGraph())
    {
        static TEString s_Current;
        s_Current = m_Evaluator.GetActiveStateName();
        return s_Current;
    }
    return s_None;
}
