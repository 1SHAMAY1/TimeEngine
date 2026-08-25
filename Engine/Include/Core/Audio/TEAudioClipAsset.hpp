#pragma once

#include "Core/Asset/Asset.hpp"
#include "Core/Audio/AudioEngine.hpp"
#include "Utils/TEString.hpp"

class TE_API TEAudioClipAsset : public Asset
{
public:
    TEAudioClipAsset();
    TEAudioClipAsset(const TEString &filePath);
    virtual ~TEAudioClipAsset();

    virtual AssetHandle GetHandle() const override { return m_Handle; }
    virtual const TEString &GetType() const override { return m_AssetTypeName; }
    virtual const TEString &GetName() const override { return m_Name; }
    virtual const TEString &GetHoverDescription() const override { return m_Description; }

    virtual TEString GetDefaultExtension() const override { return ".teaudio"; }
    virtual TEString GetDefaultIconPath() const override { return "Resources/Editor/AudioIcon.png"; }

    virtual TERef<Asset> Clone() const override;
    virtual bool LoadFromFile(const TEString &path) override;
    virtual void OnContentBrowserCreate(const TEString &path) override;

    AudioClipHandle GetAudioClipHandle() const { return m_ClipHandle; }
    const TEString &GetSourceFilePath() const { return m_SourceFilePath; }
    float GetDuration() const { return m_Duration; }
    uint32_t GetSampleRate() const { return m_SampleRate; }
    uint32_t GetChannels() const { return m_Channels; }

    void SetName(const TEString &name) { m_Name = name; }
    void SetHandle(AssetHandle handle) { m_Handle = handle; }

private:
    AssetHandle m_Handle = 0;
    AudioClipHandle m_ClipHandle = InvalidAudioClip;
    TEString m_Name = "NewAudioClip";
    TEString m_AssetTypeName = "AudioClip";
    TEString m_Description = "TimeEngine sound asset (.wav, .ogg, .mp3, .flac, .teaudio)";
    TEString m_SourceFilePath;

    float m_Duration = 0.0f;
    uint32_t m_SampleRate = 44100;
    uint32_t m_Channels = 2;
};
