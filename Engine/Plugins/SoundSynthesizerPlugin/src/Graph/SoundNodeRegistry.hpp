#pragma once

#include "Nodes/DSPNodes.hpp"
#include "Nodes/TriggerSequencerNode.hpp"
#include "Nodes/WavePlayerNode.hpp"
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace SoundStudio {

struct SoundNodeInfo
{
    TEString TypeName;
    TEString Category;
    std::function<TERef<SoundGraphNode>()> Factory;
};

class SoundNodeRegistry
{
public:
    static void RegisterNode(const TEString &typeName, const TEString &category,
                             std::function<TERef<SoundGraphNode>()> factory);

    static TERef<SoundGraphNode> CreateNode(const TEString &typeName);
    static const TEMap<TEString, SoundNodeInfo> &GetRegisteredNodes();
    static void InitializeDefaults();
};

} // namespace SoundStudio
