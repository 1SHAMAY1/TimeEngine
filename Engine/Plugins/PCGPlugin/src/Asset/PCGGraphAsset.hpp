#pragma once

#include "Core/Asset/Asset.hpp"
#include "Graph/PCGGraph.hpp"

class PCGGraphAsset : public Asset
{
public:
    PCGGraphAsset();
    virtual ~PCGGraphAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override;
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override;

    virtual TEString GetDefaultExtension() const override { return ".tepcg"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/PCGIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    PCGGraph &GetGraph() { return m_Graph; }
    const PCGGraph &GetGraph() const { return m_Graph; }

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewPCGGraph";
    PCGGraph m_Graph;
};
