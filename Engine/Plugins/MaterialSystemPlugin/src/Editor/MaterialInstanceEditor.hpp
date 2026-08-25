#pragma once

#include "Core/PreRequisites.h"
#include "Editor/AssetEditor.hpp"


class MaterialInstanceEditor : public AssetEditor
{
public:
    virtual ~MaterialInstanceEditor() override = default;

    virtual TEString GetAssetType() const override { return "MaterialInstanceAsset"; }
    virtual TEString GetAssetExtension() const override { return ".tematinst"; }
    virtual TEString GetAssetCategory() const override { return "Rendering"; }
    virtual TEString GetAssetDescription() const override { return "Parameter override instance of parent material"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"MaterialInstance\": {\n    \"Name\": \"" + name + "\",\n    \"Parent\": \"\",\n    \"Scalars\": {},\n    \"Vectors\": {}\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;
};
