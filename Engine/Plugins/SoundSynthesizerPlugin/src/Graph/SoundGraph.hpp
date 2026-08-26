#pragma once

#include "SoundGraphNode.hpp"
#include <map>
#include <memory>
#include <vector>

namespace SoundStudio
{

class SoundGraph
{
public:
    SoundGraph();
    ~SoundGraph();

    void Clear();

    uint64_t AddNode(TERef<SoundGraphNode> node);
    void RemoveNode(uint64_t nodeId);
    TERef<SoundGraphNode> GetNode(uint64_t nodeId);
    const TEMap<uint64_t, TERef<SoundGraphNode>> &GetNodes() const { return m_Nodes; }

    uint64_t ConnectPins(uint64_t outputPinId, uint64_t inputPinId);
    void DisconnectPins(uint64_t connectionId);
    void DisconnectPin(uint64_t pinId);
    const TEArray<SoundConnection> &GetConnections() const { return m_Connections; }

    SoundPin *FindPin(uint64_t pinId);

    // Topological sorting of nodes for DSP execution order
    TEArray<TERef<SoundGraphNode>> GetExecutionOrder();

    // Dynamic Parameter Modulations (e.g. SetParam("Cutoff", 1200.0f))
    void SetParameterFloat(const TEString &paramName, float value);
    float GetParameterFloat(const TEString &paramName) const;
    const TEMap<TEString, float> &GetParameters() const { return m_Parameters; }

    void ResetAllNodes();

private:
    TEMap<uint64_t, TERef<SoundGraphNode>> m_Nodes;
    TEArray<SoundConnection> m_Connections;
    TEMap<TEString, float> m_Parameters;
    uint64_t m_NextNodeId = 1;
    uint64_t m_NextConnectionId = 1;
};

} // namespace SoundStudio
