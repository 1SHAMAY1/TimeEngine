#pragma once

#include "Core/Asset/Asset.hpp"
#include "UI/UIWidget.hpp"
#include "UI/Widgets/UIContainers.hpp"
#include "Utils/TEString.hpp"

struct UIWidgetNodeDef
{
    TEString Type = "UIBorder"; // UIBorder, UIButton, UISlider, UIScrollBox, UISizeBox, UIText, UIImage
    TEString ID = "Widget";
    TEVector2 Position = {0.0f, 0.0f};
    TEVector2 Size = {100.0f, 40.0f};
    TEString Text = "";
    TEVector4 Color = {1.0f, 1.0f, 1.0f, 1.0f};
    float Value = 0.0f;
    float MinValue = 0.0f;
    float MaxValue = 1.0f;
    int ParentIndex = -1;
};

class TE_API UIAsset : public Asset
{
public:
    UIAsset();
    UIAsset(const TEString &name);
    virtual ~UIAsset() override = default;

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".teui"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual bool SaveToFile(const TEString &path);
    virtual void OnContentBrowserCreate(const TEString &path) override;

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

    const TEArray<UIWidgetNodeDef> &GetNodes() const { return m_Nodes; }
    void AddNode(const UIWidgetNodeDef &node) { m_Nodes.Add(node); }
    void ClearNodes() { m_Nodes.Clear(); }

    TERef<UIWidget> InstantiateWidgetTree() const;

private:
    AssetHandle m_Handle = 0;
    TEString m_Name = "NewUIAsset";
    TEString m_AssetTypeName = "UIAsset";
    TEString m_Description = "TimeEngine UI Layout Asset (.teui)";
    TEArray<UIWidgetNodeDef> m_Nodes;
};
