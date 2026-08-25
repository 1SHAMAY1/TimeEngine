#pragma once

#include "Core/Audio/AudioEngine.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"

class AudioSource2DComponent;
class AudioListenerComponent;

class TE_API AudioWorld
{
public:
    AudioWorld() = default;
    ~AudioWorld();

    void OnRuntimeStart();
    void OnRuntimeStop();
    void OnUpdate(float dt);

    void RegisterSource(AudioSource2DComponent *source);
    void UnregisterSource(AudioSource2DComponent *source);

    void SetListener(AudioListenerComponent *listener);
    void RemoveListener(AudioListenerComponent *listener);
    AudioListenerComponent *GetListener() const { return m_Listener; }

private:
    TEArray<AudioSource2DComponent *> m_Sources;
    AudioListenerComponent *m_Listener = nullptr;
};
