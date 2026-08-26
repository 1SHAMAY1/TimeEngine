#pragma once

#include "Editor/AssetEditor.hpp"
#include "Utils/TEString.hpp"

class TE_API AudioAssetEditor : public AssetEditor
{
public:
    AudioAssetEditor();
    virtual ~AudioAssetEditor() override = default;

    virtual TEString GetAssetType() const override { return "Audio"; }
    virtual TEString GetAssetExtension() const override { return ".teaudio"; }
    virtual TEArray<TEString> GetSupportedExtensions() const override { return {".teaudio"}; }
    virtual TEString GetAssetCategory() const override { return "Audio"; }
    virtual TEString GetAssetDescription() const override { return "Audio Sound Clip & Spatial Asset"; }
    virtual TEString CreateDefaultTemplate(const TEString &name) const override;

    virtual void DrawEditor(EditorTab &tab) override;
    virtual void DrawIcon(const TEVector2 &min, const TEVector2 &max) const override;

private:
    float m_ClipVolume = 1.0f;
    float m_ClipPitch = 1.0f;
    bool m_ClipLoop = false;
    int m_SelectedDeviceIndex = 0;
};
