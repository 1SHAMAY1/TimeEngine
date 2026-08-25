#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"


class ConstantFloatNode : public GraphNode
{
public:
    ConstantFloatNode()
    {
        NodeType = "ConstantFloat";
        Title = "Constant Float";
        Category = "Inputs/Constants";
        HeaderColor = TEColor(0.25f, 0.45f, 0.35f, 1.0f);
        Size = {160.0f, 75.0f};

        SetProperty("Value", "1.0");
        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float(), PinDirection::Output));
    }
};

class ConstantColorNode : public GraphNode
{
public:
    ConstantColorNode()
    {
        NodeType = "ConstantColor";
        Title = "Constant Color";
        Category = "Inputs/Constants";
        HeaderColor = TEColor(0.45f, 0.25f, 0.45f, 1.0f);
        Size = {170.0f, 120.0f};

        SetProperty("R", "1.0");
        SetProperty("G", "1.0");
        SetProperty("B", "1.0");
        SetProperty("A", "1.0");

        OutputPins.Add(GraphPin(0, 0, "RGBA", GraphPinType::Float4(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "RGB", GraphPinType::Float3(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "R", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "G", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float(), PinDirection::Output));
    }
};

class ScalarParameterNode : public GraphNode
{
public:
    ScalarParameterNode()
    {
        NodeType = "ScalarParameter";
        Title = "Scalar Parameter";
        Category = "Parameters";
        HeaderColor = TEColor(0.2f, 0.5f, 0.4f, 1.0f);
        Size = {180.0f, 85.0f};

        SetProperty("ParamName", "Param_Scalar");
        SetProperty("DefaultValue", "1.0");

        OutputPins.Add(GraphPin(0, 0, "Out", GraphPinType::Float(), PinDirection::Output));
    }
};

class VectorParameterNode : public GraphNode
{
public:
    VectorParameterNode()
    {
        NodeType = "VectorParameter";
        Title = "Vector Parameter";
        Category = "Parameters";
        HeaderColor = TEColor(0.45f, 0.35f, 0.55f, 1.0f);
        Size = {190.0f, 130.0f};

        SetProperty("ParamName", "Param_Color");
        SetProperty("DefaultR", "1.0");
        SetProperty("DefaultG", "1.0");
        SetProperty("DefaultB", "1.0");
        SetProperty("DefaultA", "1.0");

        OutputPins.Add(GraphPin(0, 0, "RGBA", GraphPinType::Float4(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "RGB", GraphPinType::Float3(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "R", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "G", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float(), PinDirection::Output));
    }
};

class Texture2DParameterNode : public GraphNode
{
public:
    Texture2DParameterNode()
    {
        NodeType = "Texture2DParameter";
        Title = "Texture Sample 2D";
        Category = "Parameters";
        HeaderColor = TEColor(0.55f, 0.35f, 0.15f, 1.0f);
        Size = {200.0f, 135.0f};

        SetProperty("ParamName", "AlbedoTexture");
        SetProperty("TexturePath", "");

        InputPins.Add(GraphPin(0, 0, "UV", GraphPinType::Float2(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "RGBA", GraphPinType::Float4(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "RGB", GraphPinType::Float3(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "R", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "G", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "B", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float(), PinDirection::Output));
    }
};

class UVCoordinatesNode : public GraphNode
{
public:
    UVCoordinatesNode()
    {
        NodeType = "UVCoordinates";
        Title = "TexCoords UV";
        Category = "Coordinates";
        HeaderColor = TEColor(0.5f, 0.45f, 0.15f, 1.0f);
        Size = {160.0f, 80.0f};

        SetProperty("TileX", "1.0");
        SetProperty("TileY", "1.0");

        OutputPins.Add(GraphPin(0, 0, "UV", GraphPinType::Float2(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "U", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "V", GraphPinType::Float(), PinDirection::Output));
    }
};

class PannerNode : public GraphNode
{
public:
    PannerNode()
    {
        NodeType = "Panner";
        Title = "Panner";
        Category = "Coordinates";
        HeaderColor = TEColor(0.45f, 0.4f, 0.2f, 1.0f);
        Size = {170.0f, 110.0f};

        SetProperty("SpeedX", "0.1");
        SetProperty("SpeedY", "0.0");

        InputPins.Add(GraphPin(0, 0, "UV", GraphPinType::Float2(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Time", GraphPinType::Float(), PinDirection::Input));
        InputPins.Add(GraphPin(0, 0, "Speed", GraphPinType::Float2(), PinDirection::Input));

        OutputPins.Add(GraphPin(0, 0, "UV", GraphPinType::Float2(), PinDirection::Output));
    }
};

class TimeNode : public GraphNode
{
public:
    TimeNode()
    {
        NodeType = "Time";
        Title = "Time";
        Category = "Utility";
        HeaderColor = TEColor(0.2f, 0.4f, 0.5f, 1.0f);
        Size = {150.0f, 90.0f};

        SetProperty("Speed", "1.0");

        OutputPins.Add(GraphPin(0, 0, "Time", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "SinTime", GraphPinType::Float(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "CosTime", GraphPinType::Float(), PinDirection::Output));
    }
};

class VertexColorNode : public GraphNode
{
public:
    VertexColorNode()
    {
        NodeType = "VertexColor";
        Title = "Vertex Color";
        Category = "Inputs/Constants";
        HeaderColor = TEColor(0.4f, 0.25f, 0.5f, 1.0f);
        Size = {160.0f, 110.0f};

        OutputPins.Add(GraphPin(0, 0, "RGBA", GraphPinType::Float4(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "RGB", GraphPinType::Float3(), PinDirection::Output));
        OutputPins.Add(GraphPin(0, 0, "A", GraphPinType::Float(), PinDirection::Output));
    }
};
