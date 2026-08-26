#pragma once

#include "../Graph/SoundGraph.hpp"
#include "Core/Asset/Asset.hpp"

namespace SoundStudio
{

class SoundGraphAsset : public Asset
{
public:
    SoundGraphAsset();
    virtual ~SoundGraphAsset() = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".tesoundgraph"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/AudioIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    TERef<SoundGraph> GetGraph() const { return m_Graph; }
    void SetGraph(TERef<SoundGraph> graph) { m_Graph = graph; }

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewSoundGraph";
    TEString m_AssetTypeName = "SoundGraph";
    TEString m_Description = "TimeEngine procedural audio DSP sound graph (.tesoundgraph)";

    TERef<SoundGraph> m_Graph;
};

} // namespace SoundStudio
