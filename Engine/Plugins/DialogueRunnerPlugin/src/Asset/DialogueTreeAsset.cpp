#include "DialogueTreeAsset.hpp"
#include "Core/Log.h"
#include "DialogueTreeSerializer.hpp"

DialogueTreeAsset::DialogueTreeAsset() { m_Graph.ResetToDefaultEntry(); }

const TEString &DialogueTreeAsset::GetType() const
{
    static TEString s_Type = "DialogueTree";
    return s_Type;
}

const TEString &DialogueTreeAsset::GetHoverDescription() const
{
    static TEString s_Desc = "Dialogue Tree & Narrative Graph Asset";
    return s_Desc;
}

TERef<Asset> DialogueTreeAsset::Clone() const
{
    auto clone = CreateRef<DialogueTreeAsset>();
    clone->m_Name = m_Name + "_Copy";
    clone->m_Graph = m_Graph;
    return clone;
}

bool DialogueTreeAsset::LoadFromFile(const TEString &path)
{
    m_Name = path.GetStem();
    return DialogueTreeSerializer::LoadFromFile(m_Graph, path);
}

bool DialogueTreeAsset::SaveToFile(const TEString &path) { return DialogueTreeSerializer::SaveToFile(m_Graph, path); }

void DialogueTreeAsset::OnContentBrowserCreate(const TEString &path)
{
    m_Name = path.GetStem();
    m_Graph.ResetToDefaultEntry();
    SaveToFile(path);
}
