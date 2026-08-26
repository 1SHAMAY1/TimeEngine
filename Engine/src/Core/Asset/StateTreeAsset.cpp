#include "Core/PreRequisites.h"
#include "Core/Asset/StateTreeAsset.hpp"
#include "Core/Graph/GraphSerializer.hpp"
#include "Core/Log.h"

StateTreeAsset::StateTreeAsset() { m_Graph = CreateRef<StateTreeGraph>(m_Name); }

StateTreeAsset::StateTreeAsset(const TEString &name) : m_Name(name) { m_Graph = CreateRef<StateTreeGraph>(m_Name); }

TERef<Asset> StateTreeAsset::Clone() const
{
    auto copy = CreateRef<StateTreeAsset>();
    copy->m_Name = m_Name;
    copy->m_Handle = m_Handle;
    if (m_Graph)
    {
        copy->m_Graph = CreateRef<StateTreeGraph>(*m_Graph);
    }
    return copy;
}

bool StateTreeAsset::LoadFromFile(const TEString &path)
{
    if (!m_Graph)
        m_Graph = CreateRef<StateTreeGraph>(m_Name);

    auto factory = [](const TEString &nodeType) -> TERef<GraphNode>
    {
        if (nodeType == "StateNode")
            return CreateRef<StateNode>();
        if (nodeType == "StateRootNode")
            return CreateRef<StateRootNode>();
        return CreateRef<GraphNode>();
    };

    GraphSerializer serializer(*m_Graph);
    bool ok = serializer.Deserialize(path, factory);
    if (ok)
    {
        TE_CORE_INFO("[StateTreeAsset] Loaded StateTree from '{0}'", path.c_str());
    }
    else
    {
        TE_CORE_WARN("[StateTreeAsset] Failed to load StateTree from '{0}'", path.c_str());
    }
    return ok;
}

bool StateTreeAsset::SaveToFile(const TEString &path)
{
    if (!m_Graph)
        return false;

    GraphSerializer serializer(*m_Graph);
    bool ok = serializer.Serialize(path);
    if (ok)
    {
        TE_CORE_INFO("[StateTreeAsset] Saved StateTree to '{0}'", path.c_str());
    }
    else
    {
        TE_CORE_WARN("[StateTreeAsset] Failed to save StateTree to '{0}'", path.c_str());
    }
    return ok;
}

void StateTreeAsset::OnContentBrowserCreate(const TEString &path)
{
    if (!m_Graph)
        m_Graph = CreateRef<StateTreeGraph>(m_Name);

    // Populate with a default Root and Patrol state
    m_Graph->Clear();
    auto root = m_Graph->CreateRootNode({50.0f, 100.0f});
    auto patrol = m_Graph->CreateState("Patrol", {300.0f, 100.0f});
    patrol->AddTickTask(CreateRef<STPlayAnimationTask>());
    m_Graph->ConnectStates(root->ID, patrol->ID);

    SaveToFile(path);
}
