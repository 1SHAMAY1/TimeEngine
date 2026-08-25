#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"


class MathAddNode : public GraphNode
{
public:
    MathAddNode()
    {
        NodeType = "MathAdd";
        Title = "Add";
        Category = "Math";
        HeaderColor = TEColor(0.35f, 0.35f, 0.45f, 1.0f);
        Size = {150.0f, 90.0f};

        SetProperty("DefaultB", "0.0");

        InputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float4(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float4(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float4(), PinDirection::Output));
    }
};

class MathMultiplyNode : public GraphNode
{
public:
    MathMultiplyNode()
    {
        NodeType = "MathMultiply";
        Title = "Multiply";
        Category = "Math";
        HeaderColor = TEColor(0.35f, 0.35f, 0.45f, 1.0f);
        Size = {150.0f, 90.0f};

        SetProperty("DefaultB", "1.0");

        InputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float4(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float4(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float4(), PinDirection::Output));
    }
};

class MathLerpNode : public GraphNode
{
public:
    MathLerpNode()
    {
        NodeType = "MathLerp";
        Title = "Linear Interpolate";
        Category = "Math";
        HeaderColor = TEColor(0.35f, 0.4f, 0.45f, 1.0f);
        Size = {170.0f, 115.0f};

        SetProperty("DefaultAlpha", "0.5");

        InputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float4(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float4(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Alpha", GraphPinType::Float(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float4(), PinDirection::Output));
    }
};

class MathClampNode : public GraphNode
{
public:
    MathClampNode()
    {
        NodeType = "MathClamp";
        Title = "Clamp";
        Category = "Math";
        HeaderColor = TEColor(0.3f, 0.35f, 0.4f, 1.0f);
        Size = {160.0f, 115.0f};

        SetProperty("Min", "0.0");
        SetProperty("Max", "1.0");

        InputPins.Add(GraphPin(0, 0, "Value", GraphPinType::Float4(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Min", GraphPinType::Float(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Max", GraphPinType::Float(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float4(), PinDirection::Output));
    }
};

class MathStepNode : public GraphNode
{
public:
    MathStepNode()
    {
        NodeType = "MathStep";
        Title = "Step";
        Category = "Math";
        HeaderColor = TEColor(0.3f, 0.35f, 0.4f, 1.0f);
        Size = {150.0f, 90.0f};

        SetProperty("Edge", "0.5");

        InputPins.Add(GraphPin(0, 0, "Edge", GraphPinType::Float(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Value", GraphPinType::Float(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float(), PinDirection::Output));
    }
};

class MathDotNode : public GraphNode
{
public:
    MathDotNode()
    {
        NodeType = "MathDot";
        Title = "Dot Product";
        Category = "Math";
        HeaderColor = TEColor(0.35f, 0.3f, 0.4f, 1.0f);
        Size = {150.0f, 90.0f};

        InputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float3(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float3(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float(), PinDirection::Output));
    }
};

class Noise2DNode : public GraphNode
{
public:
    Noise2DNode()
    {
        NodeType = "Noise2D";
        Title = "Simplex Noise 2D";
        Category = "Utility";
        HeaderColor = TEColor(0.25f, 0.35f, 0.45f, 1.0f);
        Size = {170.0f, 95.0f};

        SetProperty("Scale", "10.0");

        InputPins.Add(GraphPin(0, 0, "UV", GraphPinType::Float2(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Scale", GraphPinType::Float(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "Noise", GraphPinType::Float(), PinDirection::Output));
    }
};
