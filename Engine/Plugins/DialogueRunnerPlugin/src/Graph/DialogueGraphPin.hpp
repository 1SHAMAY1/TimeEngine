#pragma once

#include "Core/Graph/GraphPin.hpp"
#include "NarrativeTypes.hpp"

// DialogueGraphPin is an extension of the unified GraphPin
struct DialogueGraphPin : public GraphPin
{
    DialogueGraphPin() = default;
    DialogueGraphPin(uint64_t id, uint64_t nodeId, const TEString &name, PinType type, PinDirection dir)
        : GraphPin(id, nodeId, name,
                   (type == PinType::Flow        ? GraphPinType::Flow()
                    : type == PinType::Condition ? GraphPinType::Bool()
                    : type == PinType::ChoiceOption
                        ? GraphPinType::Custom("ChoiceOption", TEColor(0.85f, 0.65f, 0.2f, 1.0f))
                        : GraphPinType::Float()),
                   dir)
    {
    }
};
