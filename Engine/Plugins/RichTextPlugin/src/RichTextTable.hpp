#pragma once

#include "Core/Asset/TEDataTable.hpp"
#include "RichTextDataAsset.hpp"


class RichTextTable : public TEDataTable
{
public:
    RichTextTable();
    RichTextTable(const TEString &name);
    virtual ~RichTextTable() = default;

    virtual TEString GetDefaultExtension() const override { return ".terichtext"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/FileIcon.png"; }

    virtual TERef<Asset> Clone() const override;

    // Style Helpers
    RichTextStyle GetStyle(const TEString &styleName, const RichTextStyle &fallbackStyle = RichTextStyle{}) const;
    void SetStyle(const TEString &styleName, const RichTextStyle &style);
    bool HasStyle(const TEString &styleName) const;
};

