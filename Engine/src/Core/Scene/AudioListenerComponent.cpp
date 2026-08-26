#include "Core/PreRequisites.h"
#include "Core/Scene/AudioListenerComponent.hpp"
#include "Core/Scene/EntityManager.hpp"
#include "Core/Scene/TransformComponent.hpp"

AudioListenerComponent::~AudioListenerComponent() {}

void AudioListenerComponent::OnInitialize() { TComponent::OnInitialize(); }

void AudioListenerComponent::OnAttach() { TComponent::OnAttach(); }

void AudioListenerComponent::OnDetach() { TComponent::OnDetach(); }

void AudioListenerComponent::Tick(float deltaTime)
{
    if (!Active)
        return;

    if (Manager && Owner)
    {
        Entity e = GetOwnerEntity();
        if (auto *transform = Manager->GetComponent<TransformComponent>(e))
        {
            TEVector2 pos = {transform->Transform.Position.x, transform->Transform.Position.y};
            AudioEngine::SetListenerPosition(pos);
        }
    }
}
