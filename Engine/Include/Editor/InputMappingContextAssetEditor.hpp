#pragma once
#include "Editor/AssetEditor.hpp"
#include "Input/InputMappingContext.hpp"

class TE_API InputMappingContextAssetEditor : public AssetEditor
{
public:
    virtual ~InputMappingContextAssetEditor() override = default;
    virtual TEString GetAssetType() const override { return "InputMappingContext"; }
    virtual TEString GetAssetExtension() const override { return ".teimc"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".teimc", ".teinputmap", ".teinput"}; }
    virtual TEString GetAssetCategory() const override { return "Input"; }
    virtual TEString GetAssetDescription() const override { return "Action and axis keybindings context"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override
    {
        return "{\n  \"InputMappingContext\": {\n    \"Name\": \"" + name + "\",\n    \"Mappings\": []\n  }\n}";
    }
    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    int m_ListeningIndex = -1; // -1 if not detecting key, index of mapping being listened to
};
