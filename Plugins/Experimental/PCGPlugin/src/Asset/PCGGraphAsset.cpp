#include "Asset/PCGGraphAsset.hpp"
#include "Asset/PCGGraphSerializer.hpp"
#include "Nodes/PCGGeneratorNodes.hpp"
#include "Nodes/PCGSpawnerNodes.hpp"

PCGGraphAsset::PCGGraphAsset() { m_Graph.SetName("New PCG Graph"); }

const TEString &PCGGraphAsset::GetType() const
{
    static TEString s_Type = "PCGGraph";
    return s_Type;
}

const TEString &PCGGraphAsset::GetHoverDescription() const
{
    static TEString s_Desc = "Procedural Content Generation (PCG) Node Graph Asset";
    return s_Desc;
}

TERef<Asset> PCGGraphAsset::Clone() const
{
    auto clone = CreateRef<PCGGraphAsset>();
    clone->m_Name = m_Name + "_Copy";
    clone->m_Graph = m_Graph;
    return clone;
}

bool PCGGraphAsset::LoadFromFile(const TEString &path)
{
    m_Name = path.GetStem();
    return PCGGraphSerializer::LoadFromFile(m_Graph, path);
}

bool PCGGraphAsset::SaveToFile(const TEString &path) { return PCGGraphSerializer::SaveToFile(m_Graph, path); }

void PCGGraphAsset::OnContentBrowserCreate(const TEString &path)
{
    m_Name = path.GetStem();
    m_Graph.ResetToDefault();

    // Default template: Grid Generator -> Entity Spawner
    auto grid = CreateRef<PCGGridGeneratorNode>();
    grid->Position = {100.0f, 150.0f};
    m_Graph.AddNode(grid);

    auto spawner = CreateRef<PCGEntitySpawnerNode>();
    spawner->Position = {400.0f, 150.0f};
    m_Graph.AddNode(spawner);

    auto *pOut = grid->FindPinByName("Out", PinDirection::Output);
    auto *pIn = spawner->FindPinByName("In", PinDirection::Input);
    if (pOut && pIn)
    {
        m_Graph.AddConnection(pOut->ID, pIn->ID);
    }

    SaveToFile(path);
}
