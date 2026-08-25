#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"


class MaterialOutputNode : public GraphNode
{
public:
    MaterialOutputNode()
    {
        NodeType = "MaterialOutput";
        Title = "Material Output";
        Category = "Output";
        HeaderColor = TEColor(0.6f, 0.2f, 0.25f, 1.0f);
        Size = {220.0f, 160.0f};

        SetProperty("BlendMode", "Translucent"); // Opaque, Masked, Translucent, Additive
        SetProperty("ShadingModel", "DefaultLit2D"); // DefaultLit2D, Unlit
        SetProperty("TwoSided", "false");

        InputPins.Add(GraphPin(0, 0, "FrontSlab", GraphPinType::Slab(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Emissive", GraphPinType::Float4(), PinDirection::Input, "0,0,0,0"));
        InputPins.Add(GraphPin(0, 0, "DisplacementUV", GraphPinType::Float2(), PinDirection::Input, "0,0"));
        InputPins.Add(GraphPin(0, 0, "AlphaClipThreshold", GraphPinType::Float(), PinDirection::Input, "0.0"));
    }
};
