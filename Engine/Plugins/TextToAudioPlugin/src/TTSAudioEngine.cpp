#include "TTSAudioEngine.hpp"
#include "Backends/WinSAPIAudioBackend.hpp"
#include "Core/Log.h"
#include "Preprocessors/TTSGameplayTagFilter.hpp"
#include "Preprocessors/TTSRichTextPreprocessor.hpp"

TTSAudioEngine &TTSAudioEngine::Get()
{
    static TTSAudioEngine s_Instance;
    return s_Instance;
}

TTSAudioEngine::TTSAudioEngine()
{
    m_Backend = CreateScope<WinSAPIAudioBackend>();
}

TTSAudioEngine::~TTSAudioEngine()
{
    Shutdown();
}

bool TTSAudioEngine::Initialize()
{
    if (m_Initialized)
        return true;

    TE_CORE_INFO("[TTSAudioEngine] Initializing TTS Audio Engine...");
    if (m_Backend)
    {
        m_Backend->Initialize();
    }

    m_Running = true;
    m_WorkerThread = std::thread(&TTSAudioEngine::WorkerThreadMain, this);
    m_Initialized = true;

    TE_CORE_INFO("[TTSAudioEngine] TTS Audio Engine initialized.");
    return true;
}

void TTSAudioEngine::Shutdown()
{
    if (!m_Initialized)
        return;

    TE_CORE_INFO("[TTSAudioEngine] Shutting down TTS Audio Engine...");
    m_Running = false;
    m_QueueCV.notify_all();

    if (m_WorkerThread.joinable())
    {
        m_WorkerThread.join();
    }

    if (m_Backend)
    {
        m_Backend->Shutdown();
    }

    m_Initialized = false;
    TE_CORE_INFO("[TTSAudioEngine] TTS Audio Engine shutdown complete.");
}

void TTSAudioEngine::Speak(const TTSSpeakRequest &request)
{
    if (!m_Initialized)
    {
        Initialize();
    }

    // 1. Gameplay Tag Filtering (check if muted)
    if (!TTSGameplayTagFilter::ShouldSpeak())
    {
        TE_CORE_WARN("[TTSAudioEngine] Speech suppressed by GameplayTag filter.");
        return;
    }

    // 2. Synchronous mode executes immediately on calling thread
    if (request.Mode == TTSSpeakMode::Sync)
    {
        TTSProcessedSpeech speech = request.EnableRichTextAcoustics
                                        ? TTSRichTextPreprocessor::ProcessText(request.Text)
                                        : TTSProcessedSpeech{request.Text, request.Text, {}, false};

        if (m_Backend)
        {
            m_IsSpeaking = true;
            m_Backend->SynthesizeAndPlay(speech, request);
            m_IsSpeaking = false;
        }
        return;
    }

    // 3. Asynchronous mode enqueues to background worker
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        if (request.Interrupt)
        {
            std::queue<TTSSpeakRequest> empty;
            std::swap(m_Queue, empty);
            if (m_Backend)
                m_Backend->Stop();
        }
        m_Queue.push(request);
    }
    m_QueueCV.notify_one();
}

void TTSAudioEngine::SpeakSimple(const TEString &text, TTSSpeakMode mode)
{
    TTSSpeakRequest req;
    req.Text = text;
    req.Mode = mode;
    Speak(req);
}

void TTSAudioEngine::StopAll()
{
    {
        std::lock_guard<std::mutex> lock(m_QueueMutex);
        std::queue<TTSSpeakRequest> empty;
        std::swap(m_Queue, empty);
    }

    if (m_Backend)
    {
        m_Backend->Stop();
    }
    m_IsSpeaking = false;
}

TEArray<TEString> TTSAudioEngine::GetAvailableVoices()
{
    if (m_Backend)
    {
        return m_Backend->GetAvailableVoices();
    }
    return {"Default Voice"};
}

bool TTSAudioEngine::SetVoice(const TEString &voiceName)
{
    if (m_Backend)
    {
        return m_Backend->SetVoice(voiceName);
    }
    return false;
}

TEString TTSAudioEngine::GetCurrentVoice() const
{
    if (m_Backend)
    {
        return m_Backend->GetCurrentVoice();
    }
    return "Default Voice";
}

size_t TTSAudioEngine::GetQueueSize()
{
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    return m_Queue.size();
}

void TTSAudioEngine::WorkerThreadMain()
{
    while (m_Running)
    {
        TTSSpeakRequest currentReq;

        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_QueueCV.wait(lock, [this]() { return !m_Running || !m_Queue.empty(); });

            if (!m_Running)
                break;

            if (!m_Queue.empty())
            {
                currentReq = m_Queue.front();
                m_Queue.pop();
            }
            else
            {
                continue;
            }
        }

        // Process Speech
        TTSProcessedSpeech speech = currentReq.EnableRichTextAcoustics
                                        ? TTSRichTextPreprocessor::ProcessText(currentReq.Text)
                                        : TTSProcessedSpeech{currentReq.Text, currentReq.Text, {}, false};

        if (m_Backend)
        {
            m_IsSpeaking = true;
            m_Backend->SynthesizeAndPlay(speech, currentReq);
            m_IsSpeaking = false;
        }
    }
}
