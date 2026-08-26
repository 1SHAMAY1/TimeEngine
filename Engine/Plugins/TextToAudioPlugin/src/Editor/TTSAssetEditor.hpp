#pragma once

#include "Editor/AssetEditor.hpp"
#include "Utils/TEString.hpp"

class TTSAssetEditor : public AssetEditor
{
public:
    TTSAssetEditor();
    virtual ~TTSAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "TTSAudio"; }
    virtual TEString GetAssetExtension() const override { return ".tetts"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".tetts"}; }
    virtual TEString GetAssetCategory() const override { return "Audio"; }
    virtual TEString GetAssetDescription() const override { return "Text-to-Speech & Acoustic Voice Asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override;

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    TEString m_InputText =
        "<color=gold>Welcome!</color> <pause time=\"400ms\"/> <pitch value=\"+20%\">Speech synthesis active.</pitch>";
    float m_Volume = 1.0f;
    float m_Pitch = 1.0f;
    float m_Rate = 1.0f;
    bool m_EnableAcoustics = true;
    int m_SelectedVoiceIndex = 0;
};
