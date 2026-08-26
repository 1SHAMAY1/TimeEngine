#include "Core/PreRequisites.h"
#include "Core/Audio/AudioWorld.hpp"
#include "Core/Scene/AudioListenerComponent.hpp"
#include "Core/Scene/AudioSource2DComponent.hpp"

#include <algorithm>

AudioWorld::~AudioWorld()
{
    m_Sources.Clear();
    m_Listener = nullptr;
}

void AudioWorld::OnRuntimeStart()
{
    for (size_t i = 0; i < m_Sources.Num(); ++i)
    {
        auto *source = m_Sources[i];
        if (source && source->PlayOnStart)
        {
            source->Play();
        }
    }
}

void AudioWorld::OnRuntimeStop()
{
    for (size_t i = 0; i < m_Sources.Num(); ++i)
    {
        auto *source = m_Sources[i];
        if (source)
        {
            source->Stop();
        }
    }
}

void AudioWorld::OnUpdate(float dt) { AudioEngine::OnUpdate(dt); }

void AudioWorld::RegisterSource(AudioSource2DComponent *source)
{
    if (!source)
        return;

    if (!m_Sources.Contains(source))
    {
        m_Sources.Add(source);
    }
}

void AudioWorld::UnregisterSource(AudioSource2DComponent *source) { m_Sources.Remove(source); }

void AudioWorld::SetListener(AudioListenerComponent *listener) { m_Listener = listener; }

void AudioWorld::RemoveListener(AudioListenerComponent *listener)
{
    if (m_Listener == listener)
    {
        m_Listener = nullptr;
    }
}
