#pragma once

#include "Core/AI/StateTreeGraph.hpp"
#include "Core/Asset/Asset.hpp"
#include "Utils/TEString.hpp"

class TE_API StateTreeAsset : public Asset
{
public:
    StateTreeAsset();
    StateTreeAsset(const TEString &name);
    virtual ~StateTreeAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".testatetree"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    TERef<StateTreeGraph> GetGraph() const { return m_Graph; }
    void SetGraph(TERef<StateTreeGraph> graph) { m_Graph = graph; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewStateTree";
    TEString m_AssetTypeName = "StateTreeAsset";
    TEString m_Description = "TimeEngine Hierarchical State Tree AI Asset";
    TERef<StateTreeGraph> m_Graph = nullptr;
};
