#include "PreRequisites.h"
#include "AudioListenerComponent.hpp"
#include "EntityManager.hpp"
#include "TransformComponent.hpp"

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
