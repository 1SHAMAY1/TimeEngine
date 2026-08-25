#pragma once

#include "Core/PreRequisites.h"
#include "Core/Graph/GraphNode.hpp"
#include "Core/PCGContext.hpp"
#include "Core/PCGPointData.hpp"

class PCGNode : public GraphNode
{
public:
    PCGNode();
    virtual ~PCGNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) = 0;

    static GraphPinType GetPCGPointPinType()
    {
        return GraphPinType::Custom("PCGPointData", TEColor(0.2f, 0.8f, 0.95f, 1.0f));
    }

    static GraphPinType GetPCGSpatialPinType()
    {
        return GraphPinType::Custom("PCGSpatial", TEColor(0.95f, 0.6f, 0.2f, 1.0f));
    }

    PCGPointDataRef GetInputPointData(PCGExecutionContext &ctx, const TEString &pinName);
    void SetOutputPointData(PCGExecutionContext &ctx, const TEString &pinName, PCGPointDataRef data);

protected:
    void AddInputPointPin(const TEString &name);
    void AddOutputPointPin(const TEString &name);
};
