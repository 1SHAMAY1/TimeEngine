#pragma once

#include "Core/PreRequisites.h"
#include "Core/Asset/Asset.hpp"
#include "Utils/MathUtils.hpp"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/TEString.hpp"


class MaterialInstanceAsset : public Asset
{
public:
    MaterialInstanceAsset(const TEString &name = "New Material Instance");
    virtual ~MaterialInstanceAsset() override = default;

    void SetParentMaterialPath(const TEString &path) { m_ParentMaterialPath = path; }
    const TEString &GetParentMaterialPath() const { return m_ParentMaterialPath; }

    void SetScalarOverride(const TEString &name, float value) { m_ScalarOverrides[name] = value; }
    float GetScalarOverride(const TEString &name, float defaultValue = 0.0f) const;
    bool HasScalarOverride(const TEString &name) const;

    void SetVectorOverride(const TEString &name, const TEVector4 &value) { m_VectorOverrides[name] = value; }
    TEVector4 GetVectorOverride(const TEString &name, const TEVector4 &defaultValue = {1, 1, 1, 1}) const;
    bool HasVectorOverride(const TEString &name) const;

    void SetTextureOverride(const TEString &name, const TEString &texturePath) { m_TextureOverrides[name] = texturePath; }
    TEString GetTextureOverride(const TEString &name) const;
    bool HasTextureOverride(const TEString &name) const;

    const TEMap<TEString, float> &GetScalarOverrides() const { return m_ScalarOverrides; }
    const TEMap<TEString, TEVector4> &GetVectorOverrides() const { return m_VectorOverrides; }
    const TEMap<TEString, TEString> &GetTextureOverrides() const { return m_TextureOverrides; }

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    // Asset Interface
    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override
    {
        static TEString s_Type = "MaterialInstanceAsset";
        return s_Type;
    }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override
    {
        static TEString s_Desc = "Material Instance with parameter overrides";
        return s_Desc;
    }
    virtual TEString GetDefaultExtension() const override { return ".tematinst"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/MaterialIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "New Material Instance";
    TEString m_ParentMaterialPath;

    TEMap<TEString, float> m_ScalarOverrides;
    TEMap<TEString, TEVector4> m_VectorOverrides;
    TEMap<TEString, TEString> m_TextureOverrides;
};
