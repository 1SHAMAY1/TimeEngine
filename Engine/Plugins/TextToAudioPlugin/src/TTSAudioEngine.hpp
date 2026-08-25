#pragma once

#include "Backends/ITTSAudioBackend.hpp"
#include "TTSAudioTypes.hpp"
#include "Utils/TEString.hpp"

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class TTSAudioEngine
{
public:
    static TTSAudioEngine &Get();

    bool Initialize();
    void Shutdown();
    bool IsInitialized() const { return m_Initialized; }

    void Speak(const TTSSpeakRequest &request);
    void SpeakSimple(const TEString &text, TTSSpeakMode mode = TTSSpeakMode::Async);
    void StopAll();

    TEArray<TEString> GetAvailableVoices();
    bool SetVoice(const TEString &voiceName);
    TEString GetCurrentVoice() const;

    bool IsSpeaking() const { return m_IsSpeaking; }
    size_t GetQueueSize();

private:
    TTSAudioEngine();
    ~TTSAudioEngine();

    void WorkerThreadMain();

private:
    TEScope<ITTSAudioBackend> m_Backend;
    std::atomic<bool> m_Initialized{false};
    std::atomic<bool> m_Running{false};
    std::atomic<bool> m_IsSpeaking{false};

    std::thread m_WorkerThread;
    std::mutex m_QueueMutex;
    std::condition_variable m_QueueCV;
    std::queue<TTSSpeakRequest> m_Queue;
};
