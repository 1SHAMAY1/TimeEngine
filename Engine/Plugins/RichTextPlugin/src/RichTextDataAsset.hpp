#pragma once

#include "Core/Asset/DataAsset.hpp"
#include "RichTextTypes.hpp"


class RichTextDataAsset : public DataAsset
{
public:
    RichTextDataAsset();
    RichTextDataAsset(const TEString &styleName);
    virtual ~RichTextDataAsset() = default;

    virtual TERef<Asset> Clone() const override;

    // Get / Set Style Struct
    RichTextStyle ToStyle() const;
    void FromStyle(const RichTextStyle &style);

    // Style Specific Accessors
    const TEString &GetStyleName() const { return m_Style.StyleName; }
    void SetStyleName(const TEString &name) { m_Style.StyleName = name; m_Name = name.ToStdString(); }

    AssetHandle GetFontAssetHandle() const { return m_Style.FontAsset; }
    void SetFontAssetHandle(AssetHandle handle) { m_Style.FontAsset = handle; }

    float GetFontSize() const { return m_Style.FontSize; }
    void SetFontSize(float size) { m_Style.FontSize = size; }

    const TEColor &GetTextColor() const { return m_Style.TextColor; }
    void SetTextColor(const TEColor &color) { m_Style.TextColor = color; }

    bool IsBold() const { return m_Style.Bold; }
    void SetBold(bool bold) { m_Style.Bold = bold; }

    bool IsItalic() const { return m_Style.Italic; }
    void SetItalic(bool italic) { m_Style.Italic = italic; }

    bool IsUnderline() const { return m_Style.Underline; }
    void SetUnderline(bool underline) { m_Style.Underline = underline; }

    bool IsStrikethrough() const { return m_Style.Strikethrough; }
    void SetStrikethrough(bool strikethrough) { m_Style.Strikethrough = strikethrough; }

    ERichTextEffect GetEffect() const { return m_Style.Effect; }
    void SetEffect(ERichTextEffect effect) { m_Style.Effect = effect; }

    const TEVector4 &GetEffectParams() const { return m_Style.EffectParams; }
    void SetEffectParams(const TEVector4 &params) { m_Style.EffectParams = params; }

    virtual TEString SerializeRowString() const override;
    virtual bool DeserializeRowString(const TEString &rowStr) override;

protected:
    RichTextStyle m_Style;
};

