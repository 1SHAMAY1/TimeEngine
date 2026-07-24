#pragma once
#include "Editor/AssetEditor.hpp"

namespace TE
{

class TE_API MaterialAssetEditor : public AssetEditor
{
public:
    virtual ~MaterialAssetEditor() override = default;
    virtual std::string GetAssetType() const override { return "Material"; }
    virtual void DrawEditor(EditorTab &tab) override;
};

} // namespace TE
