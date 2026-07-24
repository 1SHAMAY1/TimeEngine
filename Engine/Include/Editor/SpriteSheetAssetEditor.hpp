#pragma once
#include "Editor/AssetEditor.hpp"

namespace TE
{

class TE_API SpriteSheetAssetEditor : public AssetEditor
{
public:
    virtual ~SpriteSheetAssetEditor() override = default;
    virtual std::string GetAssetType() const override { return "SpriteSheet"; }
    virtual void DrawEditor(EditorTab &tab) override;
};

} // namespace TE
