#pragma once

#include "Core/Asset/Asset.hpp"
#include "Graph/DialogueGraph.hpp"

class DialogueTreeAsset : public Asset
{
public:
    DialogueTreeAsset();
    virtual ~DialogueTreeAsset() = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override;
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override;

    virtual TEString GetDefaultExtension() const override { return ".tedialogue"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/DialogueIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

    DialogueGraph &GetGraph() { return m_Graph; }
    const DialogueGraph &GetGraph() const { return m_Graph; }

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewDialogueTree";
    DialogueGraph m_Graph;
};
