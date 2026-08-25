#pragma once

#include "../TTSAudioTypes.hpp"
#include "Utils/TEString.hpp"
#include <vector>

class ITTSAudioBackend
{
public:
    virtual ~ITTSAudioBackend() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual bool IsInitialized() const = 0;

    virtual bool SynthesizeAndPlay(const TTSProcessedSpeech &speech, const TTSSpeakRequest &request) = 0;
    virtual void Stop() = 0;

    virtual TEArray<TEString> GetAvailableVoices() = 0;
    virtual bool SetVoice(const TEString &voiceName) = 0;
    virtual TEString GetCurrentVoice() const = 0;
};
