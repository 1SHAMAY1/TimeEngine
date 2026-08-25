#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"


class HorizontalBlendNode : public GraphNode
{
public:
    HorizontalBlendNode()
    {
        NodeType = "HorizontalBlend";
        Title = "Horizontal Blend";
        Category = "Slab Operators";
        HeaderColor = TEColor(0.25f, 0.45f, 0.55f, 1.0f);
        Size = {200.0f, 130.0f};

        SetProperty("DefaultWeight", "0.5");

        InputPins.Add(GraphPin(0, 0, "SlabA", GraphPinType::Slab(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "SlabB", GraphPinType::Slab(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Weight", GraphPinType::Float(), PinDirection::Input, "0.5"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};

class VerticalLayerNode : public GraphNode
{
public:
    VerticalLayerNode()
    {
        NodeType = "VerticalLayer";
        Title = "Vertical Layer";
        Category = "Slab Operators";
        HeaderColor = TEColor(0.25f, 0.5f, 0.55f, 1.0f);
        Size = {200.0f, 130.0f};

        SetProperty("DefaultCoverage", "1.0");

        InputPins.Add(GraphPin(0, 0, "BaseSlab", GraphPinType::Slab(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "TopSlab", GraphPinType::Slab(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Coverage", GraphPinType::Float(), PinDirection::Input, "1.0"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};

class AddBlendNode : public GraphNode
{
public:
    AddBlendNode()
    {
        NodeType = "AddBlend";
        Title = "Add Blend";
        Category = "Slab Operators";
        HeaderColor = TEColor(0.35f, 0.45f, 0.55f, 1.0f);
        Size = {190.0f, 110.0f};

        InputPins.Add(GraphPin(0, 0, "SlabA", GraphPinType::Slab(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "SlabB", GraphPinType::Slab(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};
