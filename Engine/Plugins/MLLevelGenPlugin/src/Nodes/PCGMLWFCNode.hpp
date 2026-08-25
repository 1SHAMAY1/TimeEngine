#pragma once

#include "Core/PreRequisites.h"
#include "Gameframework/GameplayUtils.hpp"
#include "Utils/TEString.hpp"

#if defined(TE_HAS_PLUGIN_PCGPLUGIN) || defined(TE_PLUGIN_PCGPLUGIN)
#include "../../../../Plugins/PCGPlugin/src/Graph/PCGNode.hpp"

class PCGMLWFCNode : public PCGNode
{
public:
    PCGMLWFCNode() = default;
    virtual ~PCGMLWFCNode() override = default;

    virtual bool Execute(PCGExecutionContext &ctx) override
    {
        // TODO: Contributor implementation - WFC constraint propagation
        return true;
    }
};
#endif
