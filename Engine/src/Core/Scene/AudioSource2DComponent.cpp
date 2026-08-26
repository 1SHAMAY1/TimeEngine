#include "Core/Scene/AudioSource2DComponent.hpp"
#include "Core/PreRequisites.h"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/TransformComponent.hpp"

AudioSource2DComponent::~AudioSource2DComponent()
{
    Stop();
    if (m_ClipHandle != InvalidAudioClip)
    {
        AudioEngine::UnloadClip(m_ClipHandle);
        m_ClipHandle = InvalidAudioClip;
    }
}

void AudioSource2DComponent::OnInitialize()
{
    TComponent::OnInitialize();
    if (!ClipPath.empty())
    {
        m_ClipHandle = AudioEngine::LoadClip(ClipPath);
    }
}

void AudioSource2DComponent::OnAttach() { TComponent::OnAttach(); }

void AudioSource2DComponent::OnDetach()
{
    Stop();
    TComponent::OnDetach();
}

void AudioSource2DComponent::Tick(float deltaTime)
{
    if (m_ClipHandle == InvalidAudioClip && !ClipPath.empty())
    {
        m_ClipHandle = AudioEngine::LoadClip(ClipPath);
    }

    if (m_ClipHandle != InvalidAudioClip && Spatial)
    {
        // Update world position from Entity transform
        if (Manager && Owner)
        {
            Entity e = GetOwnerEntity();
            if (auto *transform = Manager->GetComponent<TransformComponent>(e))
            {
                TEVector2 pos = {transform->Transform.Position.x, transform->Transform.Position.y};
                AudioEngine::SetClipPosition(m_ClipHandle, pos);
            }
        }
    }
}

void AudioSource2DComponent::Play(AudioPlayMode mode)
{
    if (ClipPath.empty())
        return;

    if (m_ClipHandle == InvalidAudioClip)
    {
        m_ClipHandle = AudioEngine::LoadClip(ClipPath);
    }

    if (m_ClipHandle != InvalidAudioClip)
    {
        AudioPlayParams params;
        params.Volume = Volume;
        params.Pitch = Pitch;
        params.Pan = Pan;
        params.Loop = Loop;
        params.Spatial = Spatial;
        params.MinDistance = MinDistance;
        params.MaxDistance = MaxDistance;
        params.RollOff = RollOff;

        if (Manager && Owner)
        {
            Entity e = GetOwnerEntity();
            if (auto *transform = Manager->GetComponent<TransformComponent>(e))
            {
                params.Position = {transform->Transform.Position.x, transform->Transform.Position.y};
            }
        }

        AudioEngine::Play(m_ClipHandle, params, mode);
        m_WasPlaying = true;
    }
}

void AudioSource2DComponent::Stop()
{
    if (m_ClipHandle != InvalidAudioClip)
    {
        AudioEngine::Stop(m_ClipHandle);
    }
    m_WasPlaying = false;
}

void AudioSource2DComponent::Pause()
{
    if (m_ClipHandle != InvalidAudioClip)
    {
        AudioEngine::Pause(m_ClipHandle);
    }
}

void AudioSource2DComponent::Resume()
{
    if (m_ClipHandle != InvalidAudioClip)
    {
        AudioEngine::Resume(m_ClipHandle);
    }
}

bool AudioSource2DComponent::IsPlaying() const
{
    if (m_ClipHandle != InvalidAudioClip)
    {
        return AudioEngine::IsPlaying(m_ClipHandle);
    }
    return false;
}
