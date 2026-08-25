#pragma once

#include "Editor/AssetEditor.hpp"
#include "RichTextAnimator.hpp"
#include "RichTextLayout.hpp"
#include "RichTextParser.hpp"
#include "RichTextRenderer.hpp"
#include "RichTextTable.hpp"


class RichTextAssetEditor : public AssetEditor
{
public:
    RichTextAssetEditor();
    virtual ~RichTextAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "RichText"; }
    virtual TEString GetAssetExtension() const override { return ".terichtext"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".terichtext"}; }
    virtual TEString GetAssetCategory() const override { return "UI"; }
    virtual TEString GetAssetDescription() const override { return "Formatted RichText & BBCode Document Asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override;

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    TEString m_InputBuffer;
    float m_WrapWidth = 600.0f;
    float m_LineSpacing = 1.2f;
    bool m_EnableAnim = true;
    float m_TypewriterProgress = 1.0f;
    float m_TotalTime = 0.0f;
    TEString m_LastClickedLink;

    RichTextLayoutResult m_Layout;
    RichTextLayoutResult m_AnimatedLayout;
    bool m_bDirty = true;
};

