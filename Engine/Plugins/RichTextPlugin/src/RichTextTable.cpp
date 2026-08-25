#include "RichTextTable.hpp"


RichTextTable::RichTextTable()
    : TEDataTable("NewRichTextTable", "RichTextDataAsset")
{
    m_AssetTypeName = "RichTextTable";
    m_Description = "TimeEngine Rich Text Style Sheet Table";
}

RichTextTable::RichTextTable(const TEString &name)
    : TEDataTable(name, "RichTextDataAsset")
{
    m_AssetTypeName = "RichTextTable";
    m_Description = "TimeEngine Rich Text Style Sheet Table";
}

TERef<Asset> RichTextTable::Clone() const
{
    auto copy = CreateRef<RichTextTable>(m_Name);
    copy->m_RowOrder = m_RowOrder;
    for (const auto &[key, val] : m_Rows)
    {
        if (val)
            copy->m_Rows[key] = std::static_pointer_cast<DataAsset>(val->Clone());
    }
    return copy;
}

RichTextStyle RichTextTable::GetStyle(const TEString &styleName, const RichTextStyle &fallbackStyle) const
{
    auto *found = m_Rows.Find(styleName);
    if (found && *found)
    {
        auto rtData = std::dynamic_pointer_cast<RichTextDataAsset>(*found);
        if (rtData)
            return rtData->ToStyle();
    }
    return fallbackStyle;
}

void RichTextTable::SetStyle(const TEString &styleName, const RichTextStyle &style)
{
    auto rtData = CreateRef<RichTextDataAsset>(styleName.ToStdString());
    rtData->FromStyle(style);
    AddRow(styleName, rtData);
}

bool RichTextTable::HasStyle(const TEString &styleName) const
{
    return ContainsRow(styleName);
}

