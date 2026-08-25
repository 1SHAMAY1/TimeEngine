#pragma once

#include "Core/Scene/ComponentRegistry.hpp"
#include "GameFrameWork/TComponent.hpp"


class DialogueComponent : public TComponent
{
public:
    GENERATED_BODY(DialogueComponent)

    T_PROPERTY(TEString, StoryAssetPath, "Story Asset Path", "")
    T_PROPERTY(TEString, InitialKnot, "Initial Knot / Entry", "")
    T_PROPERTY(TEString, SpeakerName, "Speaker Name", "NPC")
    T_PROPERTY(bool, bAutoTrigger, "Auto Trigger On Proximity", false)
    T_PROPERTY(float, InteractionDistance, "Interaction Distance", 3.0f)
    T_PROPERTY(bool, bIsActive, "Is Active", true)

    virtual TEString GetClassName() const override { return StaticClassName; }
};

#ifdef TE_EDITOR
T_REGISTER_COMPONENT(DialogueComponent, "Dialogue Component")
T_REGISTER_PROPERTY(DialogueComponent, TEString, StoryAssetPath, "Story Asset Path")
T_REGISTER_PROPERTY(DialogueComponent, TEString, InitialKnot, "Initial Knot / Entry")
T_REGISTER_PROPERTY(DialogueComponent, TEString, SpeakerName, "Speaker Name")
T_REGISTER_PROPERTY(DialogueComponent, bool, bAutoTrigger, "Auto Trigger On Proximity")
T_REGISTER_PROPERTY(DialogueComponent, float, InteractionDistance, "Interaction Distance")
T_REGISTER_PROPERTY(DialogueComponent, bool, bIsActive, "Is Active")
#endif

