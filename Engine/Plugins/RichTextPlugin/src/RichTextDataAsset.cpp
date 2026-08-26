#include "RichTextDataAsset.hpp"
#include <sstream>

RichTextDataAsset::RichTextDataAsset() : DataAsset("NewRichTextStyle", "RichTextDataAsset")
{
    m_AssetTypeName = "RichTextDataAsset";
    m_Style.StyleName = "NewRichTextStyle";
}

RichTextDataAsset::RichTextDataAsset(const TEString &styleName) : DataAsset(styleName, "RichTextDataAsset")
{
    m_AssetTypeName = "RichTextDataAsset";
    m_Style.StyleName = TEString(styleName);
}

TERef<Asset> RichTextDataAsset::Clone() const
{
    auto copy = CreateRef<RichTextDataAsset>(m_Style.StyleName.ToStdString());
    copy->m_Style = m_Style;
    copy->m_Properties = m_Properties;
    return copy;
}

RichTextStyle RichTextDataAsset::ToStyle() const { return m_Style; }

void RichTextDataAsset::FromStyle(const RichTextStyle &style)
{
    m_Style = style;
    m_Name = style.StyleName.ToStdString();
}

TEString RichTextDataAsset::SerializeRowString() const
{
    auto color = m_Style.TextColor.GetValue();
    auto grad = m_Style.GradientTo.GetValue();

    return TEString(std::to_string(m_Style.FontAsset).c_str()) + "|" + TEString::FromFloat(m_Style.FontSize) + "|" +
           TEString::FromFloat(m_Style.Scale) + "|" + TEString::FromFloat(color.r) + " " +
           TEString::FromFloat(color.g) + " " + TEString::FromFloat(color.b) + " " + TEString::FromFloat(color.a) +
           "|" + (m_Style.IsGradient ? "1|" : "0|") + TEString::FromFloat(grad.r) + " " + TEString::FromFloat(grad.g) +
           " " + TEString::FromFloat(grad.b) + " " + TEString::FromFloat(grad.a) + "|" +
           TEString::FromInt(m_Style.GradientDirection) + "|" + (m_Style.Bold ? "1|" : "0|") +
           (m_Style.Italic ? "1|" : "0|") + (m_Style.Underline ? "1|" : "0|") + (m_Style.Strikethrough ? "1|" : "0|") +
           TEString::FromInt(static_cast<int>(m_Style.Effect)) + "|" + TEString::FromFloat(m_Style.EffectParams.x) +
           " " + TEString::FromFloat(m_Style.EffectParams.y) + " " + TEString::FromFloat(m_Style.EffectParams.z) + " " +
           TEString::FromFloat(m_Style.EffectParams.w) + "|" + TEString(std::to_string(m_Style.IconTexture).c_str()) +
           "|" + m_Style.SpriteName + "|" + m_Style.LinkID;
}

bool RichTextDataAsset::DeserializeRowString(const TEString &rowStr)
{
    TEArray<TEString> parts = rowStr.Split('|');
    if (parts.Num() >= 16)
    {
        m_Style.FontAsset = std::stoull(parts[0]);
        m_Style.FontSize = parts[1].ToFloat();
        m_Style.Scale = parts[2].ToFloat();

        auto colParts = parts[3].Split(' ');
        if (colParts.Num() >= 4)
        {
            m_Style.TextColor =
                TEColor(colParts[0].ToFloat(), colParts[1].ToFloat(), colParts[2].ToFloat(), colParts[3].ToFloat());
        }

        m_Style.IsGradient = (parts[4] == "1" || parts[4] == "true");

        auto gradParts = parts[5].Split(' ');
        if (gradParts.Num() >= 4)
        {
            m_Style.GradientTo =
                TEColor(gradParts[0].ToFloat(), gradParts[1].ToFloat(), gradParts[2].ToFloat(), gradParts[3].ToFloat());
        }

        m_Style.GradientDirection = std::stoi(parts[6]);
        m_Style.Bold = (parts[7] == "1" || parts[7] == "true");
        m_Style.Italic = (parts[8] == "1" || parts[8] == "true");
        m_Style.Underline = (parts[9] == "1" || parts[9] == "true");
        m_Style.Strikethrough = (parts[10] == "1" || parts[10] == "true");
        m_Style.Effect = static_cast<ERichTextEffect>(std::stoi(parts[11]));

        auto effParts = parts[12].Split(' ');
        if (effParts.Num() >= 4)
        {
            m_Style.EffectParams =
                TEVector4(effParts[0].ToFloat(), effParts[1].ToFloat(), effParts[2].ToFloat(), effParts[3].ToFloat());
        }

        m_Style.IconTexture = std::stoull(parts[13]);
        m_Style.SpriteName = parts[14];
        m_Style.LinkID = parts[15];

        return true;
    }

    return false;
}
