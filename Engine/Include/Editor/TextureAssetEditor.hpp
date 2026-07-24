#pragma once
#include "Editor/AssetEditor.hpp"

namespace TE
{

class TE_API TextureAssetEditor : public AssetEditor
{
public:
    virtual ~TextureAssetEditor() override = default;
    virtual std::string GetAssetType() const override { return "Texture"; }
    virtual void DrawEditor(EditorTab &tab) override;
};

} // namespace TE
