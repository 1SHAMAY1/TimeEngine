#pragma once
#include "Editor/AssetEditor.hpp"

namespace TE
{

class TE_API SpriteAssetEditor : public AssetEditor
{
public:
    virtual ~SpriteAssetEditor() override = default;
    virtual std::string GetAssetType() const override { return "Sprite"; }
    virtual void DrawEditor(EditorTab &tab) override;
};

} // namespace TE
