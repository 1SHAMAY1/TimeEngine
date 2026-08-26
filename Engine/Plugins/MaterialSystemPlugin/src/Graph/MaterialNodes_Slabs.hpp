#pragma once

#include "Core/Graph/GraphNode.hpp"
#include "Core/PreRequisites.h"

class SurfaceSlabNode : public GraphNode
{
public:
    SurfaceSlabNode()
    {
        NodeType = "SurfaceSlab";
        Title = "Surface Slab";
        Category = "Material Slabs";
        HeaderColor = TEColor(0.15f, 0.45f, 0.65f, 1.0f);
        Size = {210.0f, 180.0f};

        SetProperty("DefaultRoughness", "0.5");
        SetProperty("DefaultMetallic", "0.0");
        SetProperty("DefaultOpacity", "1.0");

        InputPins.Add(GraphPin(0, 0, "BaseColor", GraphPinType::Float4(), PinDirection::Input, "1,1,1,1"));
        InputPins.Add(GraphPin(0, 0, "Roughness", GraphPinType::Float(), PinDirection::Input, "0.5"));
        InputPins.Add(GraphPin(0, 0, "Metallic", GraphPinType::Float(), PinDirection::Input, "0.0"));
        InputPins.Add(GraphPin(0, 0, "Normal", GraphPinType::Float3(), PinDirection::Input, "0,0,1"));
        InputPins.Add(GraphPin(0, 0, "Height", GraphPinType::Float(), PinDirection::Input, "0.0"));
        InputPins.Add(GraphPin(0, 0, "Opacity", GraphPinType::Float(), PinDirection::Input, "1.0"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};

class CoatSlabNode : public GraphNode
{
public:
    CoatSlabNode()
    {
        NodeType = "CoatSlab";
        Title = "Coat Layer Slab";
        Category = "Material Slabs";
        HeaderColor = TEColor(0.2f, 0.55f, 0.55f, 1.0f);
        Size = {200.0f, 140.0f};

        SetProperty("DefaultRoughness", "0.1");
        SetProperty("DefaultWeight", "1.0");

        InputPins.Add(GraphPin(0, 0, "CoatColor", GraphPinType::Float4(), PinDirection::Input, "1,1,1,1"));
        InputPins.Add(GraphPin(0, 0, "CoatRoughness", GraphPinType::Float(), PinDirection::Input, "0.1"));
        InputPins.Add(GraphPin(0, 0, "CoatNormal", GraphPinType::Float3(), PinDirection::Input, "0,0,1"));
        InputPins.Add(GraphPin(0, 0, "Weight", GraphPinType::Float(), PinDirection::Input, "1.0"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};

class EmissiveSlabNode : public GraphNode
{
public:
    EmissiveSlabNode()
    {
        NodeType = "EmissiveSlab";
        Title = "Emissive Glow Slab";
        Category = "Material Slabs";
        HeaderColor = TEColor(0.65f, 0.45f, 0.15f, 1.0f);
        Size = {200.0f, 130.0f};

        SetProperty("DefaultIntensity", "1.0");

        InputPins.Add(GraphPin(0, 0, "EmissiveColor", GraphPinType::Float4(), PinDirection::Input, "1,1,1,1"));
        InputPins.Add(GraphPin(0, 0, "Intensity", GraphPinType::Float(), PinDirection::Input, "1.0"));
        InputPins.Add(GraphPin(0, 0, "PulseSpeed", GraphPinType::Float(), PinDirection::Input, "0.0"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};

class DissolveSlabNode : public GraphNode
{
public:
    DissolveSlabNode()
    {
        NodeType = "DissolveSlab";
        Title = "Dissolve Burn Slab";
        Category = "Material Slabs";
        HeaderColor = TEColor(0.65f, 0.25f, 0.25f, 1.0f);
        Size = {210.0f, 150.0f};

        SetProperty("DefaultThreshold", "0.5");
        SetProperty("DefaultEdgeWidth", "0.05");

        InputPins.Add(GraphPin(0, 0, "Threshold", GraphPinType::Float(), PinDirection::Input, "0.5"));
        InputPins.Add(GraphPin(0, 0, "EdgeWidth", GraphPinType::Float(), PinDirection::Input, "0.05"));
        InputPins.Add(GraphPin(0, 0, "EdgeColor", GraphPinType::Float4(), PinDirection::Input, "1,0.4,0.1,1"));
        InputPins.Add(GraphPin(0, 0, "NoiseMask", GraphPinType::Float(), PinDirection::Input, "0.5"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};

class UnlitSlabNode : public GraphNode
{
public:
    UnlitSlabNode()
    {
        NodeType = "UnlitSlab";
        Title = "Unlit Sprite Slab";
        Category = "Material Slabs";
        HeaderColor = TEColor(0.35f, 0.45f, 0.35f, 1.0f);
        Size = {190.0f, 110.0f};

        InputPins.Add(GraphPin(0, 0, "Color", GraphPinType::Float4(), PinDirection::Input, "1,1,1,1"));
        InputPins.Add(GraphPin(0, 0, "Opacity", GraphPinType::Float(), PinDirection::Input, "1.0"));

        OutputPins.Add(GraphPin(0, 0, "Slab", GraphPinType::Slab(), PinDirection::Output));
    }
};
