#pragma once

#include "Core/Audio/AudioEngine.hpp"
#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"

class TE_API AudioListenerComponent : public TComponent
{
public:
    GENERATED_BODY(AudioListenerComponent)

    T_PROPERTY(bool, Active, "Active Listener", true)

    AudioListenerComponent() = default;
    virtual ~AudioListenerComponent();

    void OnInitialize() override;
    void OnAttach() override;
    void OnDetach() override;
    void Tick(float deltaTime) override;

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(AudioListenerComponent, "Audio Listener Component")
T_REGISTER_PROPERTY(AudioListenerComponent, bool, Active, "Active Listener")
T_REGISTER_PRESET(AudioListenerComponent, "Audio Listener", "Audio",
                  [](EntityID id, EntityManager *em) { em->AddComponent<AudioListenerComponent>(id); })
#endif
