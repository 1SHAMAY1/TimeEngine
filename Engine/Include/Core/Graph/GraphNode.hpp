#pragma once

#include "Core/Graph/GraphPin.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include "Utils/TEString.hpp"

class TE_API GraphNode
{
public:
    GraphNode() = default;
    virtual ~GraphNode() = default;

    uint64_t ID = 0;
    TEString Title = "Node";
    TEString Category = "General";
    TEString NodeType = "GraphNode";
    TEVector2 Position = {0.0f, 0.0f};
    TEVector2 Size = {180.0f, 100.0f};
    TEColor HeaderColor = TEColor(0.2f, 0.25f, 0.35f, 1.0f);

    TEArray<GraphPin> InputPins;
    TEArray<GraphPin> OutputPins;
    TEMap<TEString, TEString> Properties;

    GraphPin *FindInputPin(uint64_t pinId);
    const GraphPin *FindInputPin(uint64_t pinId) const;

    GraphPin *FindOutputPin(uint64_t pinId);
    const GraphPin *FindOutputPin(uint64_t pinId) const;

    GraphPin *FindPinByName(const TEString &name, PinDirection direction);
    const GraphPin *FindPinByName(const TEString &name, PinDirection direction) const;

    void SetProperty(const TEString &key, const TEString &value);
    TEString GetProperty(const TEString &key, const TEString &defaultValue = "") const;
    bool HasProperty(const TEString &key) const;

    virtual void OnCreated() {}
    virtual void OnPinConnected(uint64_t pinId, uint64_t targetPinId) {}
    virtual void OnPinDisconnected(uint64_t pinId) {}

    virtual TERef<GraphNode> Clone() const;
};
