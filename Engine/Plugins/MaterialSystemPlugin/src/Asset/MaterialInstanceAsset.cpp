#include "Asset/MaterialInstanceAsset.hpp"
#include "Asset/MaterialAssetSerializer.hpp"


MaterialInstanceAsset::MaterialInstanceAsset(const TEString &name) : m_Name(name) {}

float MaterialInstanceAsset::GetScalarOverride(const TEString &name, float defaultValue) const
{
    auto it = m_ScalarOverrides.find(name);
    if (it != m_ScalarOverrides.end())
        return it->second;
    return defaultValue;
}

bool MaterialInstanceAsset::HasScalarOverride(const TEString &name) const
{
    return m_ScalarOverrides.find(name) != m_ScalarOverrides.end();
}

TEVector4 MaterialInstanceAsset::GetVectorOverride(const TEString &name, const TEVector4 &defaultValue) const
{
    auto it = m_VectorOverrides.find(name);
    if (it != m_VectorOverrides.end())
        return it->second;
    return defaultValue;
}

bool MaterialInstanceAsset::HasVectorOverride(const TEString &name) const
{
    return m_VectorOverrides.find(name) != m_VectorOverrides.end();
}

TEString MaterialInstanceAsset::GetTextureOverride(const TEString &name) const
{
    auto it = m_TextureOverrides.find(name);
    if (it != m_TextureOverrides.end())
        return it->second;
    return "";
}

bool MaterialInstanceAsset::HasTextureOverride(const TEString &name) const
{
    return m_TextureOverrides.find(name) != m_TextureOverrides.end();
}

TERef<Asset> MaterialInstanceAsset::Clone() const
{
    auto copy = CreateRef<MaterialInstanceAsset>(m_Name);
    copy->m_Handle = m_Handle;
    copy->m_ParentMaterialPath = m_ParentMaterialPath;
    copy->m_ScalarOverrides = m_ScalarOverrides;
    copy->m_VectorOverrides = m_VectorOverrides;
    copy->m_TextureOverrides = m_TextureOverrides;
    return copy;
}

bool MaterialInstanceAsset::LoadFromFile(const TEString &path)
{
    return MaterialAssetSerializer::DeserializeInstance(*this, path);
}

void MaterialInstanceAsset::OnContentBrowserCreate(const TEString &path)
{
    MaterialAssetSerializer::SerializeInstance(*this, path);
}
