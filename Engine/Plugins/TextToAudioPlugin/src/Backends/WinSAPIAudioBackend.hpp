#pragma once

#include "Core/PreRequisites.h"
#include "ITTSAudioBackend.hpp"
#include <memory>
#include <mutex>

#include <vector>

class WinSAPIAudioBackend : public ITTSAudioBackend
{
public:
    WinSAPIAudioBackend();
    virtual ~WinSAPIAudioBackend();

    bool Initialize() override;
    void Shutdown() override;
    bool IsInitialized() const override { return m_Initialized; }

    bool SynthesizeAndPlay(const TTSProcessedSpeech &speech, const TTSSpeakRequest &request) override;
    void Stop() override;

    TEArray<TEString> GetAvailableVoices() override;
    bool SetVoice(const TEString &voiceName) override;
    TEString GetCurrentVoice() const override { return m_CurrentVoice; }

private:
    bool m_Initialized = false;
    bool m_CoInitialized = false;
    void *m_pVoice = nullptr; // ISpVoice*
    TEString m_CurrentVoice = "Default";
    std::mutex m_Mutex;
};
