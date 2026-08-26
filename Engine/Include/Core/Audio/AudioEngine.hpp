#pragma once

#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"
#include <cstdint>

// Opaque audio clip handle (registered audio file in memory or disk stream)
using AudioClipHandle = uint64_t;
static constexpr AudioClipHandle InvalidAudioClip = 0;

// Attenuation curve models for spatial 2D/3D audio
enum class AudioAttenuationModel
{
    Inverse = 0,
    Linear,
    Exponential
};

// Playback options for a single sound instance
struct TE_API AudioPlayParams
{
    float Volume = 1.0f;               // 0.0f to 1.0f (or higher for boost)
    float Pitch = 1.0f;                // Playback speed multiplier (1.0 = normal)
    float Pan = 0.0f;                  // Stereo pan (-1.0 = Left, 0.0 = Center, +1.0 = Right)
    bool Loop = false;                 // Repeat sound continuously
    bool Spatial = false;              // Enable distance attenuation & panning based on position
    TEVector2 Position = {0.0f, 0.0f}; // 2D World space emitter position
    float MinDistance = 1.0f;          // Distance where attenuation begins
    float MaxDistance = 50.0f;         // Distance where sound becomes silent
    float RollOff = 1.0f;              // Attenuation curve steepness
    AudioAttenuationModel Attenuation = AudioAttenuationModel::Inverse;
};

// Playback mode: non-blocking async or blocking sync
enum class AudioPlayMode
{
    Async = 0,
    Sync
};

class TE_API AudioEngine
{
public:
    static bool Initialize();
    static void Shutdown();
    static bool IsInitialized();

    // Clip loading & management
    static AudioClipHandle LoadClip(const TEString &filePath, bool streamFromDisk = false);
    static void UnloadClip(AudioClipHandle handle);
    static void UnloadAllClips();

    // Playback controls (sound instance level)
    static void Play(AudioClipHandle handle, const AudioPlayParams &params = AudioPlayParams{},
                     AudioPlayMode mode = AudioPlayMode::Async);
    static void Stop(AudioClipHandle handle);
    static void StopAll();
    static void Pause(AudioClipHandle handle);
    static void Resume(AudioClipHandle handle);
    static bool IsPlaying(AudioClipHandle handle);

    static void SetClipVolume(AudioClipHandle handle, float volume);
    static void SetClipPitch(AudioClipHandle handle, float pitch);
    static void SetClipPosition(AudioClipHandle handle, const TEVector2 &pos);
    static void SetClipLoop(AudioClipHandle handle, bool loop);

    // Procedural PCM Submission (used by TTS, synthesized instruments, audio generators)
    static void SubmitPCMFrames(const void *pFrames, uint64_t frameCount, uint32_t sampleRate = 44100,
                                uint32_t channels = 2);

    // Global mix & category controls
    static void SetMasterVolume(float volume);
    static float GetMasterVolume();

    static void SetMusicVolume(float volume);
    static float GetMusicVolume();

    static void SetSFXVolume(float volume);
    static float GetSFXVolume();

    static void SetVoiceVolume(float volume);
    static float GetVoiceVolume();

    // Listener transformation (synced by AudioListenerComponent / active camera)
    static void SetListenerPosition(const TEVector2 &position);
    static TEVector2 GetListenerPosition();

    static void SetListenerVelocity(const TEVector2 &velocity);
    static TEVector2 GetListenerVelocity();

    // Hardware Audio Device enumeration and selection
    static TEArray<TEString> GetOutputDevices();
    static bool SetOutputDevice(const TEString &deviceName);
    static TEString GetCurrentOutputDevice();

    // Time Dilation factor hook (pitch modulates proportionally when time slows/speeds up)
    static void SetTimeDilationFactor(float factor);
    static float GetTimeDilationFactor();

    // Diagnostics
    static size_t GetActiveSoundCount();
    static size_t GetLoadedClipCount();

    // Per-frame tick (called from Application::Run or Application::OnUpdate)
    static void OnUpdate(float dt);
};
