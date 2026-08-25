#include "Graph/PCGNode.hpp"

PCGNode::PCGNode()
{
    NodeType = "PCGNode";
    Category = "PCG";
    HeaderColor = TEColor(0.2f, 0.5f, 0.7f, 1.0f);
    Size = {200.0f, 120.0f};
}

void PCGNode::AddInputPointPin(const TEString &name)
{
    InputPins.Add(GraphPin(0, ID, name, GetPCGPointPinType(), PinDirection::Input));
}

void PCGNode::AddOutputPointPin(const TEString &name)
{
    OutputPins.Add(GraphPin(0, ID, name, GetPCGPointPinType(), PinDirection::Output));
}

PCGPointDataRef PCGNode::GetInputPointData(PCGExecutionContext &ctx, const TEString &pinName)
{
    const GraphPin *pin = FindPinByName(pinName, PinDirection::Input);
    if (!pin)
        return nullptr;

    return ctx.GetPinData(pin->ID);
}

void PCGNode::SetOutputPointData(PCGExecutionContext &ctx, const TEString &pinName, PCGPointDataRef data)
{
    const GraphPin *pin = FindPinByName(pinName, PinDirection::Output);
    if (!pin)
        return;

    ctx.SetPinData(pin->ID, data);
}
