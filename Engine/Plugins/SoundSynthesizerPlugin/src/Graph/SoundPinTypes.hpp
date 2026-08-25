#pragma once

#include "../SoundStudioTypes.hpp"
#include "Renderer/TEColor.hpp"
#include "Utils/TEString.hpp"

namespace SoundStudio {

struct SoundPin
{
    uint64_t ID = 0;
    uint64_t NodeID = 0;
    TEString Name;
    EAudioPinType Type = EAudioPinType::AudioSignal;
    bool IsInput = true;

    // Default static value if unlinked
    float DefaultFloat = 0.0f;
    int DefaultInt = 0;

    // Cached live buffer for audio rate connections
    AudioBufferBlock Buffer;
    bool TriggerFired = false;

    TEColor GetPinColor() const
    {
        switch (Type)
        {
        case EAudioPinType::AudioSignal:
            return TEColor(0.2f, 0.9f, 0.4f, 1.0f); // Bright Green
        case EAudioPinType::Trigger:
            return TEColor(0.95f, 0.85f, 0.2f, 1.0f); // Bright Yellow
        case EAudioPinType::Float:
            return TEColor(0.3f, 0.7f, 0.95f, 1.0f); // Blue
        case EAudioPinType::Int:
            return TEColor(0.4f, 0.4f, 0.9f, 1.0f); // Indigo
        case EAudioPinType::Enum:
        default:
            return TEColor(0.85f, 0.45f, 0.9f, 1.0f); // Purple
        }
    }
};

struct SoundConnection
{
    uint64_t ID = 0;
    uint64_t OutputPinID = 0;
    uint64_t InputPinID = 0;
};

} // namespace SoundStudio
