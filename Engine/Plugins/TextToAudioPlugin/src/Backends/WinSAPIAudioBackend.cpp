#include "WinSAPIAudioBackend.hpp"
#include "Core/Log.h"

#ifdef TE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <cguid.h>
#include <windows.h>
#if defined(__MINGW32__)
#include <initguid.h>
#endif
#include <sapi.h>
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
#include <sphelper.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#pragma comment(lib, "sapi.lib")
#pragma comment(lib, "ole32.lib")
#endif

WinSAPIAudioBackend::WinSAPIAudioBackend() {}

WinSAPIAudioBackend::~WinSAPIAudioBackend() { Shutdown(); }

bool WinSAPIAudioBackend::Initialize()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Initialized)
        return true;

#ifdef TE_PLATFORM_WINDOWS
    HRESULT hr = CoInitialize(nullptr);
    if (SUCCEEDED(hr))
    {
        m_CoInitialized = true;
    }
    else if (hr == RPC_E_CHANGED_MODE)
    {
        m_CoInitialized = false; // COM already initialized in MTA mode, safe to use
    }
    else
    {
        TE_CORE_WARN("[WinSAPIAudioBackend] CoInitialize returned code: {0}", (long)hr);
    }

    ISpVoice *pVoice = nullptr;
    hr = CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void **)&pVoice);
    if (SUCCEEDED(hr) && pVoice)
    {
        m_pVoice = pVoice;
        m_Initialized = true;
        TE_CORE_INFO("[WinSAPIAudioBackend] Windows SAPI 5 Voice Engine initialized successfully.");
        return true;
    }
    else
    {
        TE_CORE_ERROR("[WinSAPIAudioBackend] Failed to create ISpVoice instance. HRESULT: {0}", (long)hr);
        return false;
    }
#else
    TE_CORE_WARN("[WinSAPIAudioBackend] Windows SAPI is only available on Windows platforms.");
    return false;
#endif
}

void WinSAPIAudioBackend::Shutdown()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Initialized)
        return;

#ifdef TE_PLATFORM_WINDOWS
    if (m_pVoice)
    {
        ISpVoice *pVoice = (ISpVoice *)m_pVoice;
        pVoice->Speak(nullptr, SPF_PURGEBEFORESPEAK, nullptr);
        pVoice->Release();
        m_pVoice = nullptr;
    }

    if (m_CoInitialized)
    {
        CoUninitialize();
        m_CoInitialized = false;
    }
#endif
    m_Initialized = false;
}

bool WinSAPIAudioBackend::SynthesizeAndPlay(const TTSProcessedSpeech &speech, const TTSSpeakRequest &request)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (!m_Initialized || !m_pVoice)
    {
        if (!Initialize())
            return false;
    }

#ifdef TE_PLATFORM_WINDOWS
    ISpVoice *pVoice = (ISpVoice *)m_pVoice;

    // Apply Rate (-10 to +10)
    long sapiRate = (long)((request.Rate - 1.0f) * 10.0f);
    sapiRate = (std::max)(-10L, (std::min)(10L, sapiRate));
    pVoice->SetRate(sapiRate);

    // Apply Volume (0 to 100)
    USHORT sapiVol = (USHORT)((std::max)(0.0f, (std::min)(1.0f, request.Volume)) * 100.0f);
    pVoice->SetVolume(sapiVol);

    // Purge previous speech if interrupt is requested
    DWORD flags = 0;
    if (request.Interrupt)
    {
        flags |= SPF_PURGEBEFORESPEAK;
    }

    if (request.Mode == TTSSpeakMode::Async)
    {
        flags |= SPF_ASYNC;
    }

    // Convert text string to wide char
    TEString textToSpeak = speech.PlainText;
    if (speech.HasAcousticMarkup && !speech.SSMLText.empty() && request.EnableRichTextAcoustics)
    {
        textToSpeak = speech.SSMLText;
        flags |= SPF_IS_XML;
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, textToSpeak.c_str(), -1, nullptr, 0);
    TEArray<uint16_t> wBuf;
    wBuf.Resize(len > 0 ? len : 1, 0);
    if (len > 0)
    {
        MultiByteToWideChar(CP_UTF8, 0, textToSpeak.c_str(), -1, reinterpret_cast<LPWSTR>(wBuf.GetData()), len);
    }
    HRESULT hr = pVoice->Speak(reinterpret_cast<LPCWSTR>(wBuf.GetData()), flags, nullptr);

    if (request.Mode == TTSSpeakMode::Sync)
    {
        pVoice->WaitUntilDone(INFINITE);
    }

    return SUCCEEDED(hr);
#else
    return false;
#endif
}

void WinSAPIAudioBackend::Stop()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
#ifdef TE_PLATFORM_WINDOWS
    if (m_pVoice)
    {
        ISpVoice *pVoice = (ISpVoice *)m_pVoice;
        pVoice->Speak(nullptr, SPF_PURGEBEFORESPEAK, nullptr);
    }
#endif
}

TEArray<TEString> WinSAPIAudioBackend::GetAvailableVoices()
{
    TEArray<TEString> voices;
    std::lock_guard<std::mutex> lock(m_Mutex);

#ifdef TE_PLATFORM_WINDOWS
    IEnumSpObjectTokens *pEnum = nullptr;
    HRESULT hr = SpEnumTokens(SPCAT_VOICES, nullptr, nullptr, &pEnum);
    if (SUCCEEDED(hr) && pEnum)
    {
        ULONG count = 0;
        pEnum->GetCount(&count);

        for (ULONG i = 0; i < count; ++i)
        {
            ISpObjectToken *pToken = nullptr;
            if (SUCCEEDED(pEnum->Next(1, &pToken, nullptr)) && pToken)
            {
                WCHAR *pDescription = nullptr;
                if (SUCCEEDED(SpGetDescription(pToken, &pDescription)) && pDescription)
                {
                    voices.push_back(TEString::FromWide(pDescription));
                    ::CoTaskMemFree(pDescription);
                }
                pToken->Release();
            }
        }
        pEnum->Release();
    }
#endif

    if (voices.empty())
    {
        voices.push_back("Default System Voice");
    }

    return voices;
}

bool WinSAPIAudioBackend::SetVoice(const TEString &voiceName)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
#ifdef TE_PLATFORM_WINDOWS
    if (!m_pVoice)
        return false;

    IEnumSpObjectTokens *pEnum = nullptr;
    HRESULT hr = SpEnumTokens(SPCAT_VOICES, nullptr, nullptr, &pEnum);
    if (SUCCEEDED(hr) && pEnum)
    {
        ULONG count = 0;
        pEnum->GetCount(&count);

        for (ULONG i = 0; i < count; ++i)
        {
            ISpObjectToken *pToken = nullptr;
            if (SUCCEEDED(pEnum->Next(1, &pToken, nullptr)) && pToken)
            {
                WCHAR *pDescription = nullptr;
                if (SUCCEEDED(SpGetDescription(pToken, &pDescription)) && pDescription)
                {
                    TEString name = TEString::FromWide(pDescription);
                    ::CoTaskMemFree(pDescription);

                    if (name == voiceName)
                    {
                        ISpVoice *pVoice = (ISpVoice *)m_pVoice;
                        pVoice->SetVoice(pToken);
                        m_CurrentVoice = voiceName;
                        pToken->Release();
                        pEnum->Release();
                        return true;
                    }
                }
                pToken->Release();
            }
        }
        pEnum->Release();
    }
#endif
    return false;
}
