#pragma once

#include "Core/Asset/UIAsset.hpp"
#include "Editor/AssetEditor.hpp"
#include "Utils/TimeGUI.hpp"

class TE_API UIAssetEditor : public AssetEditor
{
public:
    UIAssetEditor();
    virtual ~UIAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "UIAsset"; }
    virtual TEString GetAssetExtension() const override { return ".teui"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".teui", ".tecanvas"}; }
    virtual TEString GetAssetCategory() const override { return "UI"; }
    virtual TEString GetAssetDescription() const override { return "Visual UI Layout & Canvas Designer"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override;

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    void RenderPalette(UIAsset &asset, TERef<UIWidget> &previewRoot, bool &isDirty, int selectedNodeIndex);
    void RenderHierarchy(UIAsset &asset, int &selectedNodeIndex);
    void RenderCanvasPreview(TERef<UIWidget> previewRoot);
    void RenderInspector(UIAsset &asset, TERef<UIWidget> &previewRoot, bool &isDirty, int selectedNodeIndex);

    int m_SelectedNodeIndex = -1;
    TERef<UIWidget> m_PreviewRoot = nullptr;
    bool m_IsDirty = false;
};
