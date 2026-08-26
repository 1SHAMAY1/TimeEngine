#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"

GraphPin *GraphNode::FindInputPin(uint64_t pinId)
{
    for (auto &pin : InputPins)
    {
        if (pin.ID == pinId)
            return &pin;
    }
    return nullptr;
}

const GraphPin *GraphNode::FindInputPin(uint64_t pinId) const
{
    for (const auto &pin : InputPins)
    {
        if (pin.ID == pinId)
            return &pin;
    }
    return nullptr;
}

GraphPin *GraphNode::FindOutputPin(uint64_t pinId)
{
    for (auto &pin : OutputPins)
    {
        if (pin.ID == pinId)
            return &pin;
    }
    return nullptr;
}

const GraphPin *GraphNode::FindOutputPin(uint64_t pinId) const
{
    for (const auto &pin : OutputPins)
    {
        if (pin.ID == pinId)
            return &pin;
    }
    return nullptr;
}

GraphPin *GraphNode::FindPinByName(const TEString &name, PinDirection direction)
{
    auto &pins = (direction == PinDirection::Input) ? InputPins : OutputPins;
    for (auto &pin : pins)
    {
        if (pin.Name == name)
            return &pin;
    }
    return nullptr;
}

const GraphPin *GraphNode::FindPinByName(const TEString &name, PinDirection direction) const
{
    const auto &pins = (direction == PinDirection::Input) ? InputPins : OutputPins;
    for (const auto &pin : pins)
    {
        if (pin.Name == name)
            return &pin;
    }
    return nullptr;
}

void GraphNode::SetProperty(const TEString &key, const TEString &value) { Properties[key] = value; }

TEString GraphNode::GetProperty(const TEString &key, const TEString &defaultValue) const
{
    auto it = Properties.find(key);
    if (it != Properties.end())
        return it->second;
    return defaultValue;
}

bool GraphNode::HasProperty(const TEString &key) const { return Properties.find(key) != Properties.end(); }

TERef<GraphNode> GraphNode::Clone() const
{
    auto copy = CreateRef<GraphNode>();
    copy->ID = this->ID;
    copy->Title = this->Title;
    copy->Category = this->Category;
    copy->NodeType = this->NodeType;
    copy->Position = this->Position;
    copy->Size = this->Size;
    copy->HeaderColor = this->HeaderColor;
    copy->InputPins = this->InputPins;
    copy->OutputPins = this->OutputPins;
    copy->Properties = this->Properties;
    return copy;
}
