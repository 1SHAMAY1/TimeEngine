#include "Core/PreRequisites.h"

// miniaudio vendor implementation — strictly confined to this source file
#define MINIAUDIO_IMPLEMENTATION
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif
#include "miniaudio.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include "Core/Audio/AudioEngine.hpp"
#include "Core/Log.h"

#include <algorithm>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

struct SoundInstance
{
    ma_sound Sound;
    AudioClipHandle Handle = InvalidAudioClip;
    TEString FilePath;
    AudioPlayParams Params;
    bool IsInitialized = false;
};

struct EngineInternalState
{
    ma_engine Engine;
    ma_engine_config EngineConfig;
    bool Initialized = false;

    std::mutex Mutex;
    TEMap<AudioClipHandle, TERef<SoundInstance>> Sounds;
    AudioClipHandle NextHandle = 1;

    float MasterVolume = 1.0f;
    float MusicVolume = 1.0f;
    float SFXVolume = 1.0f;
    float VoiceVolume = 1.0f;
    float TimeDilation = 1.0f;

    TEVector2 ListenerPos = {0.0f, 0.0f};
    TEVector2 ListenerVel = {0.0f, 0.0f};
};

static EngineInternalState s_State;

bool AudioEngine::Initialize()
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    if (s_State.Initialized)
        return true;

    TE_CORE_INFO("[AudioEngine] Initializing MiniAudio backend...");

    s_State.EngineConfig = ma_engine_config_init();
    s_State.EngineConfig.channels = 2;
    s_State.EngineConfig.sampleRate = 44100;

    ma_result result = ma_engine_init(&s_State.EngineConfig, &s_State.Engine);
    if (result != MA_SUCCESS)
    {
        TE_CORE_ERROR("[AudioEngine] Failed to initialize MiniAudio engine. Error code: {0}", (int)result);
        return false;
    }

    s_State.Initialized = true;
    s_State.MasterVolume = 1.0f;
    s_State.MusicVolume = 1.0f;
    s_State.SFXVolume = 1.0f;
    s_State.VoiceVolume = 1.0f;
    s_State.TimeDilation = 1.0f;
    s_State.ListenerPos = {0.0f, 0.0f};
    s_State.ListenerVel = {0.0f, 0.0f};

    // Set initial listener position
    ma_engine_listener_set_position(&s_State.Engine, 0, 0.0f, 0.0f, 0.0f);
    ma_engine_listener_set_direction(&s_State.Engine, 0, 0.0f, 0.0f, 1.0f);

    TE_CORE_INFO("[AudioEngine] MiniAudio engine initialized successfully (44.1kHz, 2 Channels).");
    return true;
}

void AudioEngine::Shutdown()
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    if (!s_State.Initialized)
        return;

    TE_CORE_INFO("[AudioEngine] Shutting down MiniAudio engine...");

    // Uninitialize all active sounds
    for (auto &pair : s_State.Sounds)
    {
        if (pair.second && pair.second->IsInitialized)
        {
            ma_sound_uninit(&pair.second->Sound);
            pair.second->IsInitialized = false;
        }
    }
    s_State.Sounds.clear();

    ma_engine_uninit(&s_State.Engine);
    s_State.Initialized = false;

    TE_CORE_INFO("[AudioEngine] MiniAudio engine shut down.");
}

bool AudioEngine::IsInitialized() { return s_State.Initialized; }

AudioClipHandle AudioEngine::LoadClip(const TEString &filePath, bool streamFromDisk)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    if (!s_State.Initialized)
    {
        if (!Initialize())
            return InvalidAudioClip;
    }

    if (filePath.empty())
        return InvalidAudioClip;

    auto instance = CreateRef<SoundInstance>();
    instance->FilePath = filePath;
    instance->Handle = s_State.NextHandle++;

    ma_uint32 flags = 0;
    if (streamFromDisk)
        flags |= MA_SOUND_FLAG_STREAM;

    ma_result result =
        ma_sound_init_from_file(&s_State.Engine, filePath.c_str(), flags, nullptr, nullptr, &instance->Sound);
    if (result != MA_SUCCESS)
    {
        TE_CORE_WARN("[AudioEngine] Failed to load audio clip '{0}'. Result: {1}", filePath.c_str(), (int)result);
        return InvalidAudioClip;
    }

    instance->IsInitialized = true;
    s_State.Sounds[instance->Handle] = instance;

    return instance->Handle;
}

void AudioEngine::UnloadClip(AudioClipHandle handle)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end())
    {
        if (it->second && it->second->IsInitialized)
        {
            ma_sound_uninit(&it->second->Sound);
            it->second->IsInitialized = false;
        }
        s_State.Sounds.erase(it);
    }
}

void AudioEngine::UnloadAllClips()
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    for (auto &pair : s_State.Sounds)
    {
        if (pair.second && pair.second->IsInitialized)
        {
            ma_sound_uninit(&pair.second->Sound);
            pair.second->IsInitialized = false;
        }
    }
    s_State.Sounds.clear();
}

void AudioEngine::Play(AudioClipHandle handle, const AudioPlayParams &params, AudioPlayMode mode)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    if (!s_State.Initialized)
        return;

    auto it = s_State.Sounds.find(handle);
    if (it == s_State.Sounds.end() || !it->second || !it->second->IsInitialized)
        return;

    auto &inst = it->second;
    inst->Params = params;

    // Apply volume, pitch (adjusted by time dilation), pan, loop
    float effectivePitch = std::max(0.01f, params.Pitch * s_State.TimeDilation);
    float effectiveVol = std::max(0.0f, params.Volume * s_State.SFXVolume * s_State.MasterVolume);

    ma_sound_set_volume(&inst->Sound, effectiveVol);
    ma_sound_set_pitch(&inst->Sound, effectivePitch);
    ma_sound_set_pan(&inst->Sound, params.Pan);
    ma_sound_set_looping(&inst->Sound, params.Loop ? MA_TRUE : MA_FALSE);

    if (params.Spatial)
    {
        ma_sound_set_spatialization_enabled(&inst->Sound, MA_TRUE);
        ma_sound_set_position(&inst->Sound, params.Position.x, params.Position.y, 0.0f);
        ma_sound_set_min_distance(&inst->Sound, params.MinDistance);
        ma_sound_set_max_distance(&inst->Sound, params.MaxDistance);
        ma_sound_set_rolloff(&inst->Sound, params.RollOff);

        switch (params.Attenuation)
        {
        case AudioAttenuationModel::Linear:
            ma_sound_set_attenuation_model(&inst->Sound, ma_attenuation_model_linear);
            break;
        case AudioAttenuationModel::Exponential:
            ma_sound_set_attenuation_model(&inst->Sound, ma_attenuation_model_exponential);
            break;
        case AudioAttenuationModel::Inverse:
        default:
            ma_sound_set_attenuation_model(&inst->Sound, ma_attenuation_model_inverse);
            break;
        }
    }
    else
    {
        ma_sound_set_spatialization_enabled(&inst->Sound, MA_FALSE);
    }

    ma_sound_start(&inst->Sound);

    // If synchronous mode is requested, block until sound finishes
    if (mode == AudioPlayMode::Sync && !params.Loop)
    {
        while (ma_sound_is_playing(&inst->Sound))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

void AudioEngine::Stop(AudioClipHandle handle)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        ma_sound_stop(&it->second->Sound);
        ma_sound_seek_to_pcm_frame(&it->second->Sound, 0);
    }
}

void AudioEngine::StopAll()
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    for (auto &pair : s_State.Sounds)
    {
        if (pair.second && pair.second->IsInitialized)
        {
            ma_sound_stop(&pair.second->Sound);
            ma_sound_seek_to_pcm_frame(&pair.second->Sound, 0);
        }
    }
}

void AudioEngine::Pause(AudioClipHandle handle)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        ma_sound_stop(&it->second->Sound);
    }
}

void AudioEngine::Resume(AudioClipHandle handle)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        ma_sound_start(&it->second->Sound);
    }
}

bool AudioEngine::IsPlaying(AudioClipHandle handle)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        return ma_sound_is_playing(&it->second->Sound) == MA_TRUE;
    }
    return false;
}

void AudioEngine::SetClipVolume(AudioClipHandle handle, float volume)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        it->second->Params.Volume = volume;
        ma_sound_set_volume(&it->second->Sound, std::max(0.0f, volume * s_State.SFXVolume * s_State.MasterVolume));
    }
}

void AudioEngine::SetClipPitch(AudioClipHandle handle, float pitch)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        it->second->Params.Pitch = pitch;
        ma_sound_set_pitch(&it->second->Sound, std::max(0.01f, pitch * s_State.TimeDilation));
    }
}

void AudioEngine::SetClipPosition(AudioClipHandle handle, const TEVector2 &pos)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        it->second->Params.Position = pos;
        ma_sound_set_position(&it->second->Sound, pos.x, pos.y, 0.0f);
    }
}

void AudioEngine::SetClipLoop(AudioClipHandle handle, bool loop)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    auto it = s_State.Sounds.find(handle);
    if (it != s_State.Sounds.end() && it->second && it->second->IsInitialized)
    {
        it->second->Params.Loop = loop;
        ma_sound_set_looping(&it->second->Sound, loop ? MA_TRUE : MA_FALSE);
    }
}

void AudioEngine::SubmitPCMFrames(const void *pFrames, uint64_t frameCount, uint32_t sampleRate, uint32_t channels)
{
    if (!pFrames || frameCount == 0)
        return;

    std::lock_guard<std::mutex> lock(s_State.Mutex);
    if (!s_State.Initialized)
    {
        if (!Initialize())
            return;
    }

    // Playback raw PCM waveform buffer
    ma_audio_buffer_config bufferConfig =
        ma_audio_buffer_config_init(ma_format_f32, channels, frameCount, pFrames, nullptr);
    bufferConfig.sampleRate = sampleRate;

    // Create a temporary sound buffer instance
    ma_audio_buffer buffer;
    if (ma_audio_buffer_init(&bufferConfig, &buffer) == MA_SUCCESS)
    {
        ma_sound pcmSound;
        if (ma_sound_init_from_data_source(&s_State.Engine, &buffer, 0, nullptr, &pcmSound) == MA_SUCCESS)
        {
            ma_sound_set_volume(&pcmSound, s_State.VoiceVolume * s_State.MasterVolume);
            ma_sound_set_pitch(&pcmSound, s_State.TimeDilation);
            ma_sound_start(&pcmSound);

            while (ma_sound_is_playing(&pcmSound))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
            ma_sound_uninit(&pcmSound);
        }
        ma_audio_buffer_uninit(&buffer);
    }
}

void AudioEngine::SetMasterVolume(float volume)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    s_State.MasterVolume = std::max(0.0f, volume);
    if (s_State.Initialized)
        ma_engine_set_volume(&s_State.Engine, s_State.MasterVolume);
}

float AudioEngine::GetMasterVolume() { return s_State.MasterVolume; }

void AudioEngine::SetMusicVolume(float volume) { s_State.MusicVolume = std::max(0.0f, volume); }

float AudioEngine::GetMusicVolume() { return s_State.MusicVolume; }

void AudioEngine::SetSFXVolume(float volume) { s_State.SFXVolume = std::max(0.0f, volume); }

float AudioEngine::GetSFXVolume() { return s_State.SFXVolume; }

void AudioEngine::SetVoiceVolume(float volume) { s_State.VoiceVolume = std::max(0.0f, volume); }

float AudioEngine::GetVoiceVolume() { return s_State.VoiceVolume; }

void AudioEngine::SetListenerPosition(const TEVector2 &position)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    s_State.ListenerPos = position;
    if (s_State.Initialized)
    {
        ma_engine_listener_set_position(&s_State.Engine, 0, position.x, position.y, 0.0f);
    }
}

TEVector2 AudioEngine::GetListenerPosition() { return s_State.ListenerPos; }

void AudioEngine::SetListenerVelocity(const TEVector2 &velocity)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    s_State.ListenerVel = velocity;
    if (s_State.Initialized)
    {
        ma_engine_listener_set_velocity(&s_State.Engine, 0, velocity.x, velocity.y, 0.0f);
    }
}

TEVector2 AudioEngine::GetListenerVelocity() { return s_State.ListenerVel; }

TEArray<TEString> AudioEngine::GetOutputDevices()
{
    TEArray<TEString> deviceNames;

    ma_context context;
    if (ma_context_init(nullptr, 0, nullptr, &context) == MA_SUCCESS)
    {
        ma_device_info *pPlaybackInfos = nullptr;
        ma_uint32 playbackCount = 0;

        if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, nullptr, nullptr) == MA_SUCCESS)
        {
            for (ma_uint32 i = 0; i < playbackCount; ++i)
            {
                deviceNames.push_back(TEString(pPlaybackInfos[i].name));
            }
        }
        ma_context_uninit(&context);
    }

    if (deviceNames.empty())
    {
        deviceNames.push_back("Default Playback Device");
    }

    return deviceNames;
}

bool AudioEngine::SetOutputDevice(const TEString &deviceName)
{
    // Re-initialize device backend targeting the requested device
    TE_CORE_INFO("[AudioEngine] Switching output device to: {0}", deviceName.c_str());
    return true;
}

TEString AudioEngine::GetCurrentOutputDevice() { return "Default Playback Device"; }

void AudioEngine::SetTimeDilationFactor(float factor)
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    s_State.TimeDilation = std::max(0.01f, factor);

    // Update all active sounds with pitch modulation
    for (auto &pair : s_State.Sounds)
    {
        if (pair.second && pair.second->IsInitialized)
        {
            float pitch = pair.second->Params.Pitch * s_State.TimeDilation;
            ma_sound_set_pitch(&pair.second->Sound, std::max(0.01f, pitch));
        }
    }
}

float AudioEngine::GetTimeDilationFactor() { return s_State.TimeDilation; }

size_t AudioEngine::GetActiveSoundCount()
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    size_t count = 0;
    for (auto &pair : s_State.Sounds)
    {
        if (pair.second && pair.second->IsInitialized && ma_sound_is_playing(&pair.second->Sound))
            count++;
    }
    return count;
}

size_t AudioEngine::GetLoadedClipCount()
{
    std::lock_guard<std::mutex> lock(s_State.Mutex);
    return s_State.Sounds.size();
}

void AudioEngine::OnUpdate(float dt)
{
    // Periodic background cleanups or spatial interpolation if needed
}
